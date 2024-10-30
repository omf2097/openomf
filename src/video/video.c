#include <SDL.h>
#include <epoxy/gl.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/opengl/object_array.h"
#include "video/opengl/remaps.h"
#include "video/opengl/render_target.h"
#include "video/opengl/shaders.h"
#include "video/opengl/shared.h"
#include "video/opengl/texture_atlas.h"
#include "video/sdl_window.h"
#include "video/vga_state.h"
#include "video/video.h"

typedef struct video_state {
    SDL_Window *window;
    SDL_GLContext *gl_context;
    texture_atlas *atlas;
    object_array *objects;
    shared *shared;
    render_target *target;
    remaps *remaps;

    GLuint palette_prog_id;
    GLuint rgba_prog_id;

    object_array_blend_mode current_blend_mode;

    int viewport_w;
    int viewport_h;

    int screen_w;
    int screen_h;
    bool fullscreen;
    bool vsync;

    bool draw_atlas;
    float fade;
    int target_move_x;
    int target_move_y;

    video_screenshot_signal screenshot_cb;
} video_state;

#define TEX_UNIT_ATLAS 0
#define TEX_UNIT_FBO 1
#define TEX_UNIT_REMAPS 2

#define PAL_BLOCK_BINDING 0

static video_state g_video_state;

int video_init(int window_w, int window_h, bool fullscreen, bool vsync) {
    g_video_state.screen_w = window_w;
    g_video_state.screen_h = window_h;
    g_video_state.fullscreen = fullscreen;
    g_video_state.vsync = vsync;
    g_video_state.fade = 1.0f;
    g_video_state.target_move_x = 0;
    g_video_state.target_move_y = 0;
    g_video_state.current_blend_mode = MODE_SET;

    if(!create_window(&g_video_state.window, window_w, window_h, fullscreen)) {
        goto error_0;
    }
    if(!create_gl_context(&g_video_state.gl_context, g_video_state.window)) {
        goto error_1;
    }
    if(!set_vsync(g_video_state.vsync)) {
        goto error_2;
    }
    if(!create_program(&g_video_state.palette_prog_id, "palette.vert", "palette.frag")) {
        goto error_2;
    }
    if(!create_program(&g_video_state.rgba_prog_id, "rgba.vert", "rgba.frag")) {
        goto error_3;
    }

    // Fetch viewport size which may be different from window size.
    SDL_GL_GetDrawableSize(g_video_state.window, &g_video_state.viewport_w, &g_video_state.viewport_h);

    // Reset background color to black.
    glClearColor(0.0, 0.0, 0.0, 1.0);

    // Create the rest of the graphics objects
    g_video_state.atlas = atlas_create(TEX_UNIT_ATLAS, 2048, 2048);
    g_video_state.objects = object_array_create(2048.0f, 2048.0f);
    g_video_state.shared = shared_create();
    g_video_state.target = render_target_create(TEX_UNIT_FBO, NATIVE_W, NATIVE_H, GL_RGB8, GL_RGB);
    g_video_state.remaps = remaps_create(TEX_UNIT_REMAPS);

    // Create orthographic projection matrix for 2d stuff.
    GLfloat projection_matrix[16];
    ortho2d(projection_matrix, 0.0f, NATIVE_W, NATIVE_H, 0.0f);

    // Activate palette program, and bind its variables now
    activate_program(g_video_state.palette_prog_id);
    bind_uniform_4fv(g_video_state.palette_prog_id, "projection", projection_matrix);
    bind_uniform_1i(g_video_state.palette_prog_id, "atlas", TEX_UNIT_ATLAS);
    bind_uniform_1i(g_video_state.palette_prog_id, "remaps", TEX_UNIT_REMAPS);

    // Activate RGBA conversion program, and bind palette etc.
    activate_program(g_video_state.rgba_prog_id);
    bind_uniform_4fv(g_video_state.rgba_prog_id, "projection", projection_matrix);
    GLuint pal_ubo_id = shared_get_block(g_video_state.shared);
    bind_uniform_block(g_video_state.rgba_prog_id, "palette", PAL_BLOCK_BINDING, pal_ubo_id);
    bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", TEX_UNIT_FBO);
    bind_uniform_1i(g_video_state.rgba_prog_id, "remaps", TEX_UNIT_REMAPS);

    INFO("OpenGL Renderer initialized!");
    return 0;

error_3:
    delete_program(g_video_state.palette_prog_id);

error_2:
    SDL_GL_DeleteContext(g_video_state.gl_context);

error_1:
    SDL_DestroyWindow(g_video_state.window);

error_0:
    return 1;
}

void video_draw_atlas(bool draw_atlas) {
    g_video_state.draw_atlas = draw_atlas;
}

void video_reinit_renderer(void) {
}

int video_reinit(int window_w, int window_h, bool fullscreen, bool vsync) {
    return 0;
}

// Called on every game tick
void video_reset_atlas(void) {
    atlas_reset(g_video_state.atlas);
}

void video_render_prepare(void) {
    object_array_prepare(g_video_state.objects);
}

static void video_set_blend_mode(object_array_blend_mode request_mode) {
    if(g_video_state.current_blend_mode == request_mode)
        return;

    if(request_mode == MODE_SET) {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else {
        glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    g_video_state.current_blend_mode = request_mode;
}

void video_area_capture(surface *sur, int x, int y, int w, int h) {
    render_target_activate(g_video_state.target);
    unsigned char *buffer = omf_calloc(1, w * h);
    glReadPixels(x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, buffer);
    surface_create_from_data_flip(sur, w, h, buffer);
    surface_set_transparency(sur, -1);
    omf_free(buffer);
}

// TODO: Use asynchronous capture + PBO here.
static void video_screenshot_capture(void) {
    SDL_Rect r = {0, 0, g_video_state.screen_w, g_video_state.screen_h};
    unsigned char *buffer = omf_malloc(r.w * r.h * 3);
    glReadPixels(r.x, r.y, r.w, r.h, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    g_video_state.screenshot_cb(&r, buffer, true); // TODO: should preferably happen in a thread.
    omf_free(buffer);
}

// Called after frame has been rendered
void video_render_finish(void) {
    object_array_finish(g_video_state.objects);

    // If palette is dirty, flush it to the texture. Note that the range is inclusive (dirty area is start <= x <= end).
    vga_index range_start, range_end;
    vga_palette *palette;
    if(vga_state_is_palette_dirty(&palette, &range_start, &range_end)) {
        shared_set_palette(g_video_state.shared, palette, range_start, range_end);
        vga_state_mark_palette_flushed();
    }

    // If remaps are dirty, do the flush. This should be pretty rare (once per scene change)
    vga_remap_tables *tables;
    if(vga_state_is_remap_dirty(&tables)) {
        remaps_update(g_video_state.remaps, tables);
        vga_state_mark_remaps_flushed();
    }

    // Set to VGA emulation state, and render to an indexed surface
    glViewport(0, 0, NATIVE_W, NATIVE_H);
    object_array_batch batch;
    object_array_begin(g_video_state.objects, &batch);
    activate_program(g_video_state.palette_prog_id);
    render_target_activate(g_video_state.target);

    object_array_blend_mode mode;
    while(object_array_get_batch(g_video_state.objects, &batch, &mode)) {
        video_set_blend_mode(mode);
        object_array_draw(g_video_state.objects, &batch);
    }

    // Disable render target, and dump its contents as RGBA to the screen.
    render_target_deactivate();
    glViewport(0, 0, g_video_state.viewport_w, g_video_state.viewport_h);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    activate_program(g_video_state.rgba_prog_id);
    if(g_video_state.draw_atlas) {
        bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", TEX_UNIT_ATLAS);
    } else {
        bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", TEX_UNIT_FBO);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Snap screenshot from the freshly rendered state.
    if(g_video_state.screenshot_cb) {
        video_screenshot_capture();
        g_video_state.screenshot_cb = NULL;
    }

    // Flip buffers. If vsync is off, we should sleep here
    // so hat our main loop doesn't eat up all cpu :)
    SDL_GL_SwapWindow(g_video_state.window);
}

void video_close(void) {
    remaps_free(&g_video_state.remaps);
    render_target_free(&g_video_state.target);
    shared_free(&g_video_state.shared);
    object_array_free(&g_video_state.objects);
    atlas_free(&g_video_state.atlas);
    delete_program(g_video_state.palette_prog_id);
    delete_program(g_video_state.rgba_prog_id);
    SDL_GL_DeleteContext(g_video_state.gl_context);
    SDL_DestroyWindow(g_video_state.window);
    INFO("Video renderer closed.");
}

void video_move_target(int x, int y) {
    g_video_state.target_move_x = x;
    g_video_state.target_move_y = y;
}

void video_get_state(int *w, int *h, int *fs, int *vsync) {
    if(w != NULL) {
        *w = g_video_state.screen_w;
    }
    if(h != NULL) {
        *h = g_video_state.screen_h;
    }
    if(fs != NULL) {
        *fs = g_video_state.fullscreen;
    }
    if(vsync != NULL) {
        *vsync = g_video_state.vsync;
    }
}

void video_set_fade(float fade) {
    g_video_state.fade = fade;
}

void video_schedule_screenshot(video_screenshot_signal callback) {
    g_video_state.screenshot_cb = callback;
}

void video_render_background(surface *sur) {
    uint16_t tx, ty, tw, th;
    if(atlas_get(g_video_state.atlas, sur, &tx, &ty, &tw, &th)) {
        object_array_add(g_video_state.objects, 0, 0, 320, 200, tx, ty, tw, th, 0, sur->transparent, 0, 0, 0, -1, 0);
    }
}

static void draw_args(video_state *state, const surface *sur, SDL_Rect *dst, int remap_offset, int remap_rounds,
                      int pal_offset, int pal_limit, unsigned int flip_mode, unsigned int options) {
    uint16_t tx, ty, tw, th;
    if(atlas_get(g_video_state.atlas, sur, &tx, &ty, &tw, &th)) {
        object_array_add(state->objects, dst->x, dst->y, dst->w, dst->h, tx, ty, tw, th, flip_mode, sur->transparent,
                         remap_offset, remap_rounds, pal_offset, pal_limit, options);
    }
}

void video_draw_full(const surface *src_surface, int x, int y, int w, int h, int remap_offset, int remap_rounds,
                     int palette_offset, int palette_limit, unsigned int flip_mode, unsigned int options) {
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    draw_args(&g_video_state, src_surface, &dst, remap_offset, remap_rounds, palette_offset, palette_limit, flip_mode,
              options);
}

void video_draw_offset(const surface *src_surface, int x, int y, int offset, int limit) {
    SDL_Rect dst;
    dst.w = src_surface->w;
    dst.h = src_surface->h;
    dst.x = x;
    dst.y = y;
    draw_args(&g_video_state, src_surface, &dst, 0, 0, offset, limit, 0, 0);
}

void video_draw_size(const surface *src_surface, int x, int y, int w, int h) {
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    draw_args(&g_video_state, src_surface, &dst, 0, 0, 0, 255, 0, 0);
}

void video_draw(const surface *src_surface, int x, int y) {
    video_draw_offset(src_surface, x, y, 0, 255);
}
