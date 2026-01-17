#include "video/renderers/opengl3/gl3_renderer.h"
#include "video/renderers/common.h"
#include "video/renderers/opengl3/sdl_window.h"

#include "video/renderers/opengl3/helpers/object_array.h"
#include "video/renderers/opengl3/helpers/remaps.h"
#include "video/renderers/opengl3/helpers/render_target.h"
#include "video/renderers/opengl3/helpers/shaders.h"
#include "video/renderers/opengl3/helpers/shared.h"
#include "video/renderers/opengl3/helpers/texture_atlas.h"

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/vga_state.h"

#define TEX_UNIT_ATLAS 0
#define TEX_UNIT_FBO 1
#define TEX_UNIT_FBO2 2
#define TEX_UNIT_REMAPS 3
#define PAL_BLOCK_BINDING 0
#define NATIVE_W 320
#define NATIVE_H 200

typedef struct gl3_context {
    SDL_Window *window;
    SDL_GLContext *gl_context;
    texture_atlas *atlas;
    object_array *objects;
    shared *shared;
    render_target *paletted_target;
    render_target *rgba_target;
    remaps *remaps;

    int viewport_w;
    int viewport_h;
    int screen_w;
    int screen_h;
    int fb_scale;
    bool fullscreen;
    bool vsync;
    int aspect;
    int scaling_mode;
    int target_move_x;
    int target_move_y;
    bool draw_atlas;
    SDL_Rect culling_area;

    uint64_t framerate_limit;
    uint64_t last_tick;

    object_array_blend_mode current_blend_mode;
    GLuint palette_prog_id;
    GLuint rgba_prog_id;
    GLuint scale_prog_id;

    video_screenshot_signal screenshot_cb;
} gl3_context;

static bool is_available(void) {
    return has_gl_available(3, 3);
}

static const char *get_description(void) {
    return "Hardware OpenGL 3.3 renderer";
}

static const char *get_name(void) {
    return "OpenGL3";
}

static void get_scaling_shader_names(const int scaling_mode, const char **vert_shader, const char **frag_shader) {
    switch(scaling_mode) {
        case 1: // Bilinear
            *vert_shader = "scalers/bilinear.vert";
            *frag_shader = "scalers/bilinear.frag";
            break;
        case 2: // CRT
            *vert_shader = "scalers/crt.vert";
            *frag_shader = "scalers/crt.frag";
            break;
        default:
            *vert_shader = "scalers/none.vert";
            *frag_shader = "scalers/none.frag";
            break;
    }
}

static void set_framerate_limit(gl3_context *ctx, int framerate_limit) {
    if(framerate_limit == 0) {
        ctx->framerate_limit = 0;
    } else {
        ctx->framerate_limit = (1.0 / framerate_limit) * SDL_GetPerformanceFrequency();
    }
}

static void reload_scaler_program(const gl3_context *ctx) {
    GLfloat projection_matrix[16];
    ortho2d(projection_matrix, 0.0f, NATIVE_W, NATIVE_H, 0.0f);
    activate_program(ctx->scale_prog_id);
    bind_uniform_4fv(ctx->scale_prog_id, "projection", projection_matrix);
    bind_uniform_1i(ctx->scale_prog_id, "framebuffer", TEX_UNIT_FBO2);
    const int fb_w = NATIVE_W * ctx->fb_scale;
    const int fb_h = NATIVE_H * ctx->fb_scale;
    bind_uniform_2f(ctx->scale_prog_id, "texture_size", (GLfloat)fb_w, (GLfloat)fb_h);
}

static bool setup_context(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync, int aspect,
                          int framerate_limit, int fb_scale, int scaling_mode) {
    gl3_context *ctx = userdata;
    ctx->screen_w = window_w;
    ctx->screen_h = window_h;
    ctx->fullscreen = fullscreen;
    ctx->framerate_limit = framerate_limit;
    ctx->fb_scale = fb_scale;
    ctx->vsync = vsync;
    ctx->aspect = aspect;
    ctx->scaling_mode = scaling_mode;
    ctx->target_move_x = 0;
    ctx->target_move_y = 0;
    ctx->current_blend_mode = MODE_SET;
    ctx->last_tick = SDL_GetPerformanceCounter();
    set_framerate_limit(ctx, framerate_limit);

    if(!create_window(&ctx->window, window_w, window_h, fullscreen)) {
        goto error_0;
    }
    if(!create_gl_context(&ctx->gl_context, ctx->window)) {
        goto error_1;
    }
    if(!set_vsync(ctx->vsync)) {
        goto error_2;
    }
    if(!create_program(&ctx->palette_prog_id, "palette.vert", "palette.frag")) {
        goto error_2;
    }
    if(!create_program(&ctx->rgba_prog_id, "rgba.vert", "rgba.frag")) {
        goto error_3;
    }
    const char *scale_vert, *scale_frag;
    get_scaling_shader_names(scaling_mode, &scale_vert, &scale_frag);
    if(!create_program(&ctx->scale_prog_id, scale_vert, scale_frag)) {
        goto error_4;
    }

    // Fetch viewport size which may be different from window size.
    SDL_GL_GetDrawableSize(ctx->window, &ctx->viewport_w, &ctx->viewport_h);

    // Reset background color to black.
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Our texture alignment is always 1, so just set it once at start.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // This is the intermediate framebuffer size.
    const int fb_w = NATIVE_W * ctx->fb_scale;
    const int fb_h = NATIVE_H * ctx->fb_scale;

    // Create the rest of the graphics objects
    ctx->atlas = atlas_create(TEX_UNIT_ATLAS, 2048, 2048);
    ctx->objects = object_array_create(2048.0f, 2048.0f);
    ctx->shared = shared_create();
    ctx->paletted_target = render_target_create(TEX_UNIT_FBO, fb_w, fb_h, GL_RGBA8, GL_RGBA, GL_NEAREST);
    ctx->rgba_target = render_target_create(TEX_UNIT_FBO2, fb_w, fb_h, GL_RGBA8, GL_RGBA, GL_NEAREST);
    ctx->remaps = remaps_create(TEX_UNIT_REMAPS);

    vga_state_mark_dirty();

    // Create orthographic projection matrix for 2d stuff.
    GLfloat projection_matrix[16];
    ortho2d(projection_matrix, 0.0f, NATIVE_W, NATIVE_H, 0.0f);

    // Activate palette program and bind its variables now
    activate_program(ctx->palette_prog_id);
    bind_uniform_4fv(ctx->palette_prog_id, "projection", projection_matrix);
    bind_uniform_1i(ctx->palette_prog_id, "atlas", TEX_UNIT_ATLAS);
    bind_uniform_1i(ctx->palette_prog_id, "remaps", TEX_UNIT_REMAPS);

    // Activate RGBA conversion program and bind palette etc.
    activate_program(ctx->rgba_prog_id);
    bind_uniform_4fv(ctx->rgba_prog_id, "projection", projection_matrix);
    GLuint pal_ubo_id = shared_get_block(ctx->shared);
    bind_uniform_block(ctx->rgba_prog_id, "palette", PAL_BLOCK_BINDING, pal_ubo_id);
    bind_uniform_1i(ctx->rgba_prog_id, "framebuffer", TEX_UNIT_FBO);
    bind_uniform_1i(ctx->rgba_prog_id, "remaps", TEX_UNIT_REMAPS);

    // Activate scale program and bind uniforms
    reload_scaler_program(ctx);

    log_info("OpenGL3 Renderer initialized!");
    return true;

error_4:
    delete_program(ctx->rgba_prog_id);

error_3:
    delete_program(ctx->palette_prog_id);

error_2:
    SDL_GL_DeleteContext(ctx->gl_context);

error_1:
    SDL_DestroyWindow(ctx->window);

error_0:
    return false;
}

static void get_context_state(void *userdata, int *window_w, int *window_h, bool *fullscreen, bool *vsync, int *aspect,
                              int *fb_scale) {
    gl3_context *ctx = userdata;
    if(window_w != NULL) {
        *window_w = ctx->screen_w;
    }
    if(window_h != NULL) {
        *window_h = ctx->screen_h;
    }
    if(fullscreen != NULL) {
        *fullscreen = ctx->fullscreen;
    }
    if(vsync != NULL) {
        *vsync = ctx->vsync;
    }
    if(aspect != NULL) {
        *aspect = ctx->aspect;
    }
    if(fb_scale != NULL) {
        *fb_scale = ctx->fb_scale;
    }
}

static bool reset_context_with(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync, int aspect,
                               int framerate_limit, int fb_scale, int scaling_mode) {
    gl3_context *ctx = userdata;
    ctx->screen_w = window_w;
    ctx->screen_h = window_h;
    ctx->fullscreen = fullscreen;
    ctx->vsync = vsync;
    ctx->aspect = aspect;

    set_framerate_limit(ctx, framerate_limit);
    bool success = resize_window(ctx->window, window_w, window_h, fullscreen);
    success = set_vsync(ctx->vsync) && success;

    const bool fb_scale_changed = ctx->fb_scale != fb_scale;
    const bool scaling_mode_changed = ctx->scaling_mode != scaling_mode;
    const int fb_w = NATIVE_W * fb_scale;
    const int fb_h = NATIVE_H * fb_scale;

    if(fb_scale_changed) {
        ctx->fb_scale = fb_scale;
        render_target_free(&ctx->paletted_target);
        render_target_free(&ctx->rgba_target);
        ctx->paletted_target = render_target_create(TEX_UNIT_FBO, fb_w, fb_h, GL_RGBA8, GL_RGBA, GL_NEAREST);
        ctx->rgba_target = render_target_create(TEX_UNIT_FBO2, fb_w, fb_h, GL_RGBA8, GL_RGBA, GL_NEAREST);
    }

    // Reload scaling shader if scaling mode changed
    if(scaling_mode_changed) {
        const char *scale_vert, *scale_frag;
        get_scaling_shader_names(scaling_mode, &scale_vert, &scale_frag);
        GLuint new_prog_id;
        if(create_program(&new_prog_id, scale_vert, scale_frag)) {
            delete_program(ctx->scale_prog_id);
            ctx->scale_prog_id = new_prog_id;
            reload_scaler_program(ctx);
            ctx->scaling_mode = scaling_mode;
        } else {
            log_error("Failed to load scaling shader, reverting to previous.");
            success = false;
        }
    } else if(fb_scale_changed) {
        // If only fb_scale changed, update shader uniforms with new texture size
        reload_scaler_program(ctx);
    }

    // Fetch viewport size which may be different from window size.
    SDL_GL_GetDrawableSize(ctx->window, &ctx->viewport_w, &ctx->viewport_h);

    log_info("OpenGL3 renderer reset.");
    return success;
}

static void reset_context(void *userdata) {
    return;
}

static void close_context(void *userdata) {
    gl3_context *ctx = userdata;
    remaps_free(&ctx->remaps);
    render_target_free(&ctx->paletted_target);
    render_target_free(&ctx->rgba_target);
    shared_free(&ctx->shared);
    object_array_free(&ctx->objects);
    atlas_free(&ctx->atlas);
    delete_program(ctx->palette_prog_id);
    delete_program(ctx->rgba_prog_id);
    delete_program(ctx->scale_prog_id);
    SDL_GL_DeleteContext(ctx->gl_context);
    SDL_DestroyWindow(ctx->window);
    log_info("OpenGL3 renderer closed.");
}

static void draw_surface(void *userdata, const surface *src_surface, SDL_Rect *dst, int remap_offset, int remap_rounds,
                         int palette_offset, int palette_limit, int opacity, unsigned int flip_mode,
                         unsigned int options) {
    const gl3_context *ctx = userdata;
    uint16_t tx, ty, tw, th;
    if(atlas_get(ctx->atlas, src_surface, &tx, &ty, &tw, &th)) {
        object_array_add(ctx->objects, dst->x, dst->y, dst->w, dst->h, tx, ty, tw, th, flip_mode,
                         src_surface->transparent, remap_offset, remap_rounds, palette_offset, palette_limit, opacity,
                         options);
    }
}

static void move_target(void *userdata, int x, int y) {
    gl3_context *ctx = userdata;
    ctx->target_move_x = x;
    ctx->target_move_y = y;
}

static void render_prepare(void *userdata, unsigned framebuffer_options) {
    const gl3_context *ctx = userdata;
    object_array_prepare(ctx->objects);

    bind_uniform_1u(ctx->rgba_prog_id, "framebuffer_options", framebuffer_options);
}

static inline void video_set_blend_mode(gl3_context *ctx, object_array_blend_mode request_mode) {
    if(ctx->current_blend_mode == request_mode) {
        return;
    }

    if(request_mode == MODE_SET) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else if(request_mode == MODE_ADD) {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    } else if(request_mode == MODE_REMAP) {
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
    } else if(request_mode == MODE_SPRITE_SHADOW) {
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_MAX);
    } else if(request_mode == MODE_DARK_TINT) {
        glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else {
        assert(!"invalid blend mode");
        return;
    }

    if(request_mode != MODE_SPRITE_SHADOW) {
        glDisable(GL_BLEND);
    }

    ctx->current_blend_mode = request_mode;
}

// TODO: Use asynchronous capture + PBO here.
static void capture_screenshot(void *userdata) {
    gl3_context *ctx = userdata;
    SDL_Rect r = {0, 0, ctx->screen_w, ctx->screen_h};
    unsigned char *buffer = omf_malloc(r.w * r.h * 3);
    glReadPixels(r.x, r.y, r.w, r.h, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    ctx->screenshot_cb(&r, buffer, true); // TODO: should preferably happen in a thread.
    omf_free(buffer);
}

#define ASPECT_X (4.0f / 3.0f)
#define ASPECT_Y (3.0f / 4.0f)

/**
 * Set the viewport, and do screen-shakes here.
 */
static inline void set_screen_viewport(const gl3_context *ctx) {
    SDL_Rect viewport = {0, 0, ctx->viewport_w, ctx->viewport_h};

    // aspect == 0 means 4:3
    // aspect == 1 means stretch to window
    if(ctx->aspect == 0) {
        const SDL_Rect window = viewport;
        find_resolution_for_aspect_ratio(&viewport, &window, 4, 3);
    }

    float move_ratio = viewport.w / NATIVE_W;
    viewport.x += ctx->target_move_x * move_ratio; // This is used for screen shakes on x-axis
    viewport.y += ctx->target_move_y * move_ratio; // This is used for screen shakes on y-axis
    glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
}

/**
 * If palette is dirty, flush it to the texture. Note that the range is inclusive (dirty area is start <= x <= end).
 */
static inline void flush_palettes(gl3_context *ctx) {
    vga_index first, last;
    vga_palette *palette;
    if(vga_state_is_palette_dirty(&palette, &first, &last)) {
        shared_set_palette(ctx->shared, palette, first, last);
        vga_state_mark_palette_flushed();
    }
}

/**
 * If remaps are dirty, do the flush. This should be pretty rare (once per scene change)
 */
static inline void flush_remaps(gl3_context *ctx) {
    vga_remap_tables *tables;
    if(vga_state_is_remap_dirty(&tables)) {
        remaps_update(ctx->remaps, tables);
        vga_state_mark_remaps_flushed();
    }
}

static inline void finish_offscreen(gl3_context *ctx) {
    object_array_finish(ctx->objects);

    // Set to VGA emulation state, and render to an indexed surface
    const int fb_w = NATIVE_W * ctx->fb_scale;
    const int fb_h = NATIVE_H * ctx->fb_scale;
    glViewport(0, 0, fb_w, fb_h);
    object_array_batch batch;
    object_array_begin(ctx->objects, &batch);
    activate_program(ctx->palette_prog_id);
    render_target_activate(ctx->paletted_target);

    object_array_blend_mode mode;
    while(object_array_get_batch(ctx->objects, &batch, &mode)) {
        video_set_blend_mode(ctx, mode);
        object_array_draw(ctx->objects, &batch);
    }
}

/**
 * Render atlas directly to screen for debugging purposes
 */
static void finish_debug_atlas(gl3_context *ctx) {
    render_target_deactivate();
    set_screen_viewport(ctx);
    activate_program(ctx->rgba_prog_id);
    bind_uniform_1i(ctx->rgba_prog_id, "framebuffer", TEX_UNIT_ATLAS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

/**
 * Convert paletted framebuffer to RGBA, then scale to screen.
 */
static inline void finish_onscreen(gl3_context *ctx) {
    video_set_blend_mode(ctx, MODE_SET);

    // Step 1: Render to RGB FBO at internal resolution
    const int fb_w = NATIVE_W * ctx->fb_scale;
    const int fb_h = NATIVE_H * ctx->fb_scale;
    glViewport(0, 0, fb_w, fb_h);
    render_target_activate(ctx->rgba_target);
    activate_program(ctx->rgba_prog_id);
    bind_uniform_1i(ctx->rgba_prog_id, "framebuffer", TEX_UNIT_FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Step 2: Scale and render onscreen
    // Filtering is handled by the scaling shader, not OpenGL
    render_target_deactivate();
    set_screen_viewport(ctx);
    activate_program(ctx->scale_prog_id);
    bind_uniform_1i(ctx->scale_prog_id, "framebuffer", TEX_UNIT_FBO2);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void render_finish(void *userdata) {
    gl3_context *ctx = userdata;
    flush_palettes(ctx);
    flush_remaps(ctx);
    if(ctx->draw_atlas) {
        finish_debug_atlas(ctx);
    } else {
        finish_offscreen(ctx);
        finish_onscreen(ctx);
    }

    // Snap screenshot from the freshly rendered state.
    if(ctx->screenshot_cb) {
        capture_screenshot(ctx);
        ctx->screenshot_cb = NULL;
    }

    // Flip buffers. If vsync is off, we should sleep here
    // so hat our main loop doesn't eat up all cpu :)
    SDL_GL_SwapWindow(ctx->window);

    // Limit framerate if requested.
    if(ctx->framerate_limit != 0) {
        uint64_t frame_time = SDL_GetPerformanceCounter() - ctx->last_tick;
        if(frame_time < ctx->framerate_limit) {
            double wait = ctx->framerate_limit - frame_time;
            double ms_conv = SDL_GetPerformanceFrequency() / 1000;
            SDL_Delay(wait / ms_conv); // TODO: SDL_DelayNS() or alternative.
        }
        ctx->last_tick = SDL_GetPerformanceCounter();
    }
}

static void render_area_prepare(void *userdata, const SDL_Rect *area) {
    gl3_context *ctx = userdata;
    object_array_prepare(ctx->objects);
    ctx->culling_area = *area;
}

static void render_area_finish(void *userdata, surface *dst) {
    gl3_context *ctx = userdata;
    finish_offscreen(ctx);
    SDL_Rect *r = &ctx->culling_area;
    unsigned char *buffer = omf_malloc(r->w * r->h);
    glReadPixels(r->x, r->y, r->w, r->h, GL_RED, GL_UNSIGNED_BYTE, buffer);
    surface_create_from_data_flip(dst, r->w, r->h, buffer);
    surface_set_transparency(dst, -1);
    omf_free(buffer);
}

static void capture_screen(void *userdata, video_screenshot_signal screenshot_cb) {
    gl3_context *ctx = userdata;
    ctx->screenshot_cb = screenshot_cb;
}

static void signal_scene_change(void *userdata) {
    gl3_context *ctx = userdata;
    atlas_reset(ctx->atlas);
}

static void signal_draw_atlas(void *userdata, bool toggle) {
    gl3_context *ctx = userdata;
    ctx->draw_atlas = toggle;
}

static void renderer_create(renderer *gl3_renderer) {
    gl3_renderer->ctx = omf_calloc(1, sizeof(gl3_context));
}

static void renderer_destroy(renderer *gl3_renderer) {
    omf_free(gl3_renderer->ctx);
}

void gl3_renderer_set_callbacks(renderer *gl3_renderer) {
    gl3_renderer->is_available = is_available;
    gl3_renderer->get_description = get_description;
    gl3_renderer->get_name = get_name;

    gl3_renderer->create = renderer_create;
    gl3_renderer->destroy = renderer_destroy;

    gl3_renderer->setup_context = setup_context;
    gl3_renderer->get_context_state = get_context_state;
    gl3_renderer->reset_context_with = reset_context_with;
    gl3_renderer->reset_context = reset_context;
    gl3_renderer->close_context = close_context;

    gl3_renderer->draw_surface = draw_surface;
    gl3_renderer->move_target = move_target;
    gl3_renderer->render_prepare = render_prepare;
    gl3_renderer->render_finish = render_finish;
    gl3_renderer->render_area_prepare = render_area_prepare;
    gl3_renderer->render_area_finish = render_area_finish;

    gl3_renderer->capture_screen = capture_screen;
    gl3_renderer->signal_scene_change = signal_scene_change;
    gl3_renderer->signal_draw_atlas = signal_draw_atlas;
}
