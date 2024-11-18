#include <SDL.h>

#include "utils/log.h"
#include "video/renderers/renderer.h"
#include "video/video.h"

// If-def the includes here
#include "video/renderers/opengl3/gl3_renderer.h"

typedef void (*renderer_init)(renderer *renderer);
typedef struct renderer_status {
    renderer_init init;
    bool is_available;
    const char *name;
    const char *description;
} renderer_status;

// If-def the renderers here. Most preferred renderers at the top.
static renderer_status renderer_statuses[] = {
    {gl3_renderer_init, false, NULL, NULL},
};

static const int renderer_count = sizeof(renderer_statuses) / sizeof(renderer_status);
static renderer current_renderer;

/**
 * This is run at start to hunt the available renderers.
 */
void video_scan_renderers(void) {
    renderer tmp;
    bool available;
    for(int i = 0; i < renderer_count; i++) {
        renderer_statuses[i].init(&tmp);
        available = tmp.is_available();
        renderer_statuses[i].is_available = available;
        renderer_statuses[i].name = tmp.get_name();
        renderer_statuses[i].description = tmp.get_description();
        DEBUG("Renderer '%s' is %s", tmp.get_name(), available ? "available" : "not available");
    }
}

bool video_get_renderer_info(int index, bool *is_available, const char **name, const char **description) {
    if(index < 0 && index >= renderer_count)
        return false;
    if(is_available != NULL)
        *is_available = renderer_statuses[index].is_available;
    if(name != NULL)
        *name = renderer_statuses[index].name;
    if(description != NULL)
        *description = renderer_statuses[index].description;
    return true;
}

int video_get_renderer_count(void) {
    return renderer_count;
}

static bool hunt_renderer_by_name(const char *try_name) {
    for(int i = 0; i < renderer_count; i++) {
        if(strcmp(renderer_statuses[i].name, try_name) != 0)
            continue;
        if(!renderer_statuses[i].is_available)
            continue;
        renderer_statuses[i].init(&current_renderer);
        return true;
    }
    return false;
}

static bool find_best_renderer(void) {
    for(int i = 0; i < renderer_count; i++) {
        if(renderer_statuses[i].is_available) {
            renderer_statuses[i].init(&current_renderer);
            return true;
        }
    }
    return false;
}

static bool video_find_renderer(const char *try_name) {
    memset(&current_renderer, 0, sizeof(renderer));
    if(try_name != NULL && strlen(try_name) > 0) {
        if(hunt_renderer_by_name(try_name)) {
            INFO("Found configured renderer '%s'!", current_renderer.get_name());
            return true;
        }
        PERROR("Unable to find specified renderer '%s', trying other alternatives ...", try_name);
    }
    if(find_best_renderer()) {
        INFO("Found available renderer '%s'!", current_renderer.get_name());
        return true;
    }
    PERROR("Unable to find any available renderer!");
    memset(&current_renderer, 0, sizeof(renderer));
    return false;
}

bool video_init(const char *try_name, int window_w, int window_h, bool fullscreen, bool vsync) {
    if(!video_find_renderer(try_name))
        goto exit_0;
    current_renderer.init(&current_renderer);
    if(!current_renderer.setup_context(current_renderer.ctx, window_w, window_h, fullscreen, vsync)) {
        goto exit_1;
    }
    return true;

exit_1:
    current_renderer.close(&current_renderer);
exit_0:
    return false;
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
    current_renderer.close(&current_renderer);
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
