#include <SDL.h>
#include <epoxy/gl.h>
#include <stdlib.h>

#include "formats/palette.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/image.h"
#include "video/opengl/object_array.h"
#include "video/opengl/remaps.h"
#include "video/opengl/render_target.h"
#include "video/opengl/shaders.h"
#include "video/opengl/shared.h"
#include "video/opengl/texture_atlas.h"
#include "video/sdl_window.h"
#include "video/video.h"

typedef struct video_state {
    SDL_Window *window;
    SDL_GLContext *gl_context;
    texture_atlas *atlas;
    object_array *objects;
    shared *shared;
    render_target *targets[2];
    remaps *remaps;

    GLuint palette_prog_id;
    GLuint rgba_prog_id;

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

    // Palettes
    palette *base_palette;          // Copy of the scenes base palette
    screen_palette *screen_palette; // Normal rendering palette
    screen_palette *extra_palette;  // Reflects base palette, used for additive blending
} video_state;

#define TEX_UNIT_ATLAS 0
#define TEX_UNIT_FBO 1
#define TEX_UNIT_FBO2 2

#define PAL_BLOCK_BINDING 0
#define REMAPS_BLOCK_BINDING 1

static const int fbo_tex_units[] = {TEX_UNIT_FBO, TEX_UNIT_FBO2};

static video_state g_video_state;

int video_init(int window_w, int window_h, bool fullscreen, bool vsync) {
    g_video_state.screen_w = window_w;
    g_video_state.screen_h = window_h;
    g_video_state.fullscreen = fullscreen;
    g_video_state.vsync = vsync;
    g_video_state.fade = 1.0f;
    g_video_state.target_move_x = 0;
    g_video_state.target_move_y = 0;

    // Clear palettes
    g_video_state.base_palette = omf_calloc(1, sizeof(palette));
    g_video_state.extra_palette = omf_calloc(1, sizeof(screen_palette));
    g_video_state.screen_palette = omf_calloc(1, sizeof(screen_palette));
    g_video_state.extra_palette->version = 0;
    g_video_state.screen_palette->version = 1;

    if(!create_window(&g_video_state.window, window_w, window_h, fullscreen)) {
        goto error_0;
    }
    if(!create_gl_context(&g_video_state.gl_context, g_video_state.window)) {
        goto error_1;
    }
    if(!enable_vsync()) {
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
    g_video_state.atlas = atlas_create(TEX_UNIT_ATLAS, 4096, 4096);
    g_video_state.objects = object_array_create(4096.0f, 4096.0f);
    g_video_state.shared = shared_create();
    g_video_state.targets[0] = render_target_create(TEX_UNIT_FBO, NATIVE_W, NATIVE_H, GL_R8, GL_RED);
    g_video_state.targets[1] = render_target_create(TEX_UNIT_FBO2, NATIVE_W, NATIVE_H, GL_R8, GL_RED);
    g_video_state.remaps = remaps_create();

    // Create orthographic projection matrix for 2d stuff.
    GLfloat projection_matrix[16];
    ortho2d(projection_matrix, 0.0f, NATIVE_W, NATIVE_H, 0.0f);

    // Activate palette program, and bind its variables now
    activate_program(g_video_state.palette_prog_id);
    bind_uniform_4fv(g_video_state.palette_prog_id, "projection", projection_matrix);
    bind_uniform_1i(g_video_state.palette_prog_id, "atlas", TEX_UNIT_ATLAS);
    GLuint remaps_ubo_id = remaps_get_block_id(g_video_state.remaps);
    bind_uniform_block(g_video_state.palette_prog_id, "remaps", REMAPS_BLOCK_BINDING, remaps_ubo_id);

    // Activate RGBA conversion program, and bind palette etc.
    activate_program(g_video_state.rgba_prog_id);
    bind_uniform_4fv(g_video_state.rgba_prog_id, "projection", projection_matrix);
    GLuint pal_ubo_id = shared_get_block(g_video_state.shared);
    bind_uniform_block(g_video_state.rgba_prog_id, "palette", PAL_BLOCK_BINDING, pal_ubo_id);
    bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", TEX_UNIT_FBO2);

    INFO("OpenGL Renderer initialized!");
    return 0;

error_3:
    delete_program(g_video_state.palette_prog_id);

error_2:
    SDL_GL_DeleteContext(g_video_state.gl_context);

error_1:
    SDL_DestroyWindow(g_video_state.window);

error_0:
    omf_free(g_video_state.screen_palette);
    omf_free(g_video_state.extra_palette);
    omf_free(g_video_state.base_palette);
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
void video_tick(void) {
}

void video_render_prepare(void) {
    memcpy(g_video_state.screen_palette->data, g_video_state.base_palette->data, 768);
    object_array_prepare(g_video_state.objects);
}

// Called after frame has been rendered
void video_render_finish(void) {
    shared_set_palette(g_video_state.shared, g_video_state.screen_palette->data);
    shared_flush_dirty(g_video_state.shared);
    remaps_update(g_video_state.remaps, (void *)g_video_state.base_palette->remaps);
    object_array_finish(g_video_state.objects);

    int target_select = 0;
    glViewport(0, 0, NATIVE_W, NATIVE_H);
    object_array_batch batch;
    object_array_begin(g_video_state.objects, &batch);
    activate_program(g_video_state.palette_prog_id);
    while(object_array_get_batch(g_video_state.objects, &batch)) {
        bind_uniform_1i(g_video_state.palette_prog_id, "framebuffer", fbo_tex_units[!target_select]);
        render_target_activate(g_video_state.targets[target_select]);
        object_array_draw(g_video_state.objects, &batch);
        target_select = !target_select;
    }

    // Disable render target, and dump its contents as RGBA to the screen.
    render_target_deactivate();
    glViewport(0, 0, g_video_state.viewport_w, g_video_state.viewport_h);
    activate_program(g_video_state.rgba_prog_id);
    if(g_video_state.draw_atlas) {
        bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", TEX_UNIT_ATLAS);
    } else {
        bind_uniform_1i(g_video_state.rgba_prog_id, "framebuffer", fbo_tex_units[!target_select]);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Flip buffers. If vsync is off, we should sleep here
    // so hat our main loop doesn't eat up all cpu :)
    SDL_GL_SwapWindow(g_video_state.window);
    if(!g_video_state.vsync) {
        SDL_Delay(1);
    }
}

void video_close(void) {
    remaps_free(&g_video_state.remaps);
    render_target_free(&g_video_state.targets[0]);
    render_target_free(&g_video_state.targets[1]);
    shared_free(&g_video_state.shared);
    object_array_free(&g_video_state.objects);
    atlas_free(&g_video_state.atlas);
    delete_program(g_video_state.palette_prog_id);
    delete_program(g_video_state.rgba_prog_id);
    SDL_GL_DeleteContext(g_video_state.gl_context);
    SDL_DestroyWindow(g_video_state.window);
    omf_free(g_video_state.screen_palette);
    omf_free(g_video_state.extra_palette);
    omf_free(g_video_state.base_palette);
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

int video_screenshot(image *img) {
    return 1;
}

int video_area_capture(surface *sur, int x, int y, int w, int h) {
    return 1;
}

void video_force_pal_refresh(void) {
    memcpy(g_video_state.screen_palette->data, g_video_state.base_palette->data, 768);
    memcpy(g_video_state.extra_palette->data, g_video_state.base_palette->data, 768);
    g_video_state.extra_palette->version++;
    g_video_state.screen_palette->version++;
}

void video_set_base_palette(const palette *src) {
    memcpy(g_video_state.base_palette, src, sizeof(palette));
    video_force_pal_refresh();
}

palette *video_get_base_palette(void) {
    return g_video_state.base_palette;
}

void video_copy_base_pal_range(const palette *src, int src_start, int dst_start, int amount) {
    memcpy(g_video_state.base_palette->data + dst_start, src->data + src_start, amount * 3);
    video_force_pal_refresh();
}

screen_palette *video_get_pal_ref(void) {
    return g_video_state.screen_palette;
}

void video_render_background(surface *sur) {
    uint16_t tx, ty, tw, th;
    if(atlas_get(g_video_state.atlas, sur, &tx, &ty, &tw, &th)) {
        object_array_add(g_video_state.objects, 0, 0, 320, 200, tx, ty, tw, th, 0, BLEND_ALPHA);
    }
}

static void render_sprite_fsot(video_state *state, surface *sur, SDL_Rect *dst, VIDEO_BLEND_MODE blend_mode,
                               int pal_offset, unsigned int flip_mode, uint8_t opacity, color color_mod) {

    uint16_t tx, ty, tw, th;
    if(atlas_get(g_video_state.atlas, sur, &tx, &ty, &tw, &th)) {
        object_array_add(state->objects, dst->x, dst->y, dst->w, dst->h, tx, ty, tw, th, flip_mode, blend_mode);
    }
}

void video_render_sprite_tint(surface *sur, int sx, int sy, color c, int pal_offset) {

    video_render_sprite_flip_scale_opacity_tint(sur, sx, sy, BLEND_ALPHA, pal_offset, FLIP_NONE, 1.0f, 1.0f, 255, c);
}

// Wrapper
void video_render_sprite(surface *sur, int sx, int sy, VIDEO_BLEND_MODE blend_mode, int pal_offset) {
    video_render_sprite_flip_scale_opacity(sur, sx, sy, blend_mode, pal_offset, FLIP_NONE, 1.0f, 1.0f, 255);
}

void video_render_sprite_flip_scale_opacity(surface *sur, int sx, int sy, VIDEO_BLEND_MODE blend_mode, int pal_offset,
                                            unsigned int flip_mode, float x_percent, float y_percent, uint8_t opacity) {

    video_render_sprite_flip_scale_opacity_tint(sur, sx, sy, blend_mode, pal_offset, flip_mode, x_percent, y_percent,
                                                opacity, color_create(0xFF, 0xFF, 0xFF, 0xFF));
}

void video_render_sprite_size(surface *sur, int sx, int sy, int sw, int sh) {
    SDL_Rect dst;
    dst.w = sw;
    dst.h = sh;
    dst.x = sx;
    dst.y = sy;

    // Render
    render_sprite_fsot(&g_video_state, sur, &dst, BLEND_ALPHA, 0, 0, 0xFF,
                       color_create(0xFF, 0xFF, 0xFF, 0xFF)); // tint
}

void video_render_sprite_flip_scale_opacity_tint(surface *sur, int sx, int sy, VIDEO_BLEND_MODE blend_mode,
                                                 int pal_offset, unsigned int flip_mode, float x_percent,
                                                 float y_percent, uint8_t opacity, color tint) {

    // Position
    SDL_Rect dst;
    dst.w = sur->w * x_percent;
    dst.h = sur->h * y_percent;
    dst.x = sx;
    dst.y = sy + (sur->h - dst.h) / 2;

    render_sprite_fsot(&g_video_state, sur, &dst, blend_mode, pal_offset, flip_mode, opacity, tint);
}
