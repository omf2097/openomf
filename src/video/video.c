#include <SDL.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"

#include "video/renderers/opengl3/gl3_renderer.h"
#include "video/renderers/renderer.h"
#include "video/video.h"

static renderer current_renderer;

bool video_init(int window_w, int window_h, bool fullscreen, bool vsync) {
    gl3_renderer_init(&current_renderer);
    return current_renderer.setup_context(current_renderer.ctx, window_w, window_h, fullscreen, vsync);
}

void video_draw_atlas(bool draw_atlas) {
    current_renderer.signal_draw_atlas(current_renderer.ctx, draw_atlas);
}

void video_reinit_renderer(void) {
    current_renderer.reset_context(current_renderer.ctx);
}

bool video_reinit(int window_w, int window_h, bool fullscreen, bool vsync) {
    return current_renderer.reset_context_with(current_renderer.ctx, window_w, window_h, fullscreen, vsync);
}

void video_signal_scene_change(void) {
    current_renderer.signal_scene_change(current_renderer.ctx);
}

void video_render_prepare(void) {
    current_renderer.render_prepare(current_renderer.ctx);
}

void video_render_finish(void) {
    current_renderer.render_finish(current_renderer.ctx);
}

void video_render_area_prepare(const SDL_Rect *area) {
    current_renderer.render_area_prepare(current_renderer.ctx, area);
}

void video_render_area_finish(surface *dst) {
    current_renderer.render_area_finish(current_renderer.ctx, dst);
}

void video_close(void) {
    current_renderer.close_context(current_renderer.ctx);
}

void video_move_target(int x, int y) {
    current_renderer.move_target(current_renderer.ctx, x, y);
}

void video_get_state(int *w, int *h, bool *fs, bool *vsync) {
    current_renderer.get_context_state(current_renderer.ctx, w, h, fs, vsync);
}

void video_schedule_screenshot(video_screenshot_signal callback) {
    current_renderer.capture_screen(current_renderer.ctx, callback);
}

static inline void draw_args(const surface *sur, SDL_Rect *dst, int remap_offset, int remap_rounds, int palette_offset,
                             int palette_limit, int opacity, unsigned int flip_mode, unsigned int options) {
    current_renderer.draw_surface(current_renderer.ctx, sur, dst, remap_offset, remap_rounds, palette_offset,
                                  palette_limit, opacity, flip_mode, options);
}

void video_draw_full(const surface *src_surface, int x, int y, int w, int h, int remap_offset, int remap_rounds,
                     int palette_offset, int palette_limit, int opacity, unsigned int flip_mode, unsigned int options) {
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    draw_args(src_surface, &dst, remap_offset, remap_rounds, palette_offset, palette_limit, opacity, flip_mode,
              options);
}

void video_draw_offset(const surface *src_surface, int x, int y, int offset, int limit) {
    SDL_Rect dst;
    dst.w = src_surface->w;
    dst.h = src_surface->h;
    dst.x = x;
    dst.y = y;
    draw_args(src_surface, &dst, 0, 0, offset, limit, 255, 0, 0);
}

void video_draw_size(const surface *src_surface, int x, int y, int w, int h) {
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    draw_args(src_surface, &dst, 0, 0, 0, 255, 255, 0, 0);
}

void video_draw(const surface *src_surface, int x, int y) {
    SDL_Rect dst;
    dst.w = src_surface->w;
    dst.h = src_surface->h;
    dst.x = x;
    dst.y = y;
    draw_args(src_surface, &dst, 0, 0, 0, 255, 255, 0, 0);
}

void video_draw_remap(const surface *src_surface, int x, int y, int remap_offset, int remap_rounds,
                      unsigned int options) {
    SDL_Rect dst;
    dst.w = src_surface->w;
    dst.h = src_surface->h;
    dst.x = x;
    dst.y = y;
    draw_args(src_surface, &dst, remap_offset, remap_rounds, 0, 255, 255, 0, options);
}
