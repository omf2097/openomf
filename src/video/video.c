#include <SDL.h>

#include "game/gui/text_render.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "video/renderers/renderer.h"
#include "video/video.h"

// If-def the includes here
#ifdef ENABLE_OPENGL3_RENDERER
#include "video/renderers/opengl3/gl3_renderer.h"
#endif
#ifdef ENABLE_NULL_RENDERER
#include "video/renderers/null/null_renderer.h"
#endif

#define MAX_AVAILABLE_RENDERERS 8

typedef void (*renderer_init)(renderer *renderer);

// This is the list of all built-in renderers.
// If-def the renderers here. Most preferred renderers at the top.
static renderer_init all_renderers[] = {
#ifdef ENABLE_OPENGL3_RENDERER
    gl3_renderer_set_callbacks,
#endif
#ifdef ENABLE_NULL_RENDERER
    null_renderer_set_callbacks,
#endif
};
static int all_renderers_count = N_ELEMENTS(all_renderers);

// All renderers that are available. This is filled by video_scan_renderers().
static struct available_renderer {
    renderer_init set_callbacks;
    const char *name;
    const char *description;
} available_renderers[MAX_AVAILABLE_RENDERERS];
static int renderer_count = 0;

// Currently selected renderer
static renderer current_renderer;

/**
 * This is run at start to hunt the available renderers.
 */
void video_scan_renderers(void) {
    renderer tmp;
    renderer_count = 0;
    for(int i = 0; i < all_renderers_count; i++) {
        all_renderers[i](&tmp);
        if(renderer_count >= MAX_AVAILABLE_RENDERERS)
            break;
        if(tmp.is_available()) {
            available_renderers[renderer_count].set_callbacks = all_renderers[i];
            available_renderers[renderer_count].name = tmp.get_name();
            available_renderers[renderer_count].description = tmp.get_description();
            DEBUG("Renderer '%s' is available", available_renderers[renderer_count].name);
            renderer_count++;
        }
    }
}

/**
 * Get information about a renderer by its index.
 * @param index Renderer index
 * @param name Renderer name
 * @param description Renderer description
 * @return true if data was read, false if there was no renderer at this index.
 */
bool video_get_renderer_info(int index, const char **name, const char **description) {
    if(index < 0 && index >= renderer_count)
        return false;
    if(name != NULL)
        *name = available_renderers[index].name;
    if(description != NULL)
        *description = available_renderers[index].description;
    return true;
}

/**
 * Get the number of currently available renderers.
 * @return Number of available renderers
 */
int video_get_renderer_count(void) {
    return renderer_count;
}

/**
 * Attempt to find the renderer by name
 * @param try_name Renderer name
 * @return true if renderer was found, false if not.
 */
static bool hunt_renderer_by_name(const char *try_name) {
    for(int i = 0; i < renderer_count; i++) {
        if(strcmp(available_renderers[i].name, try_name) != 0)
            continue;
        available_renderers[i].set_callbacks(&current_renderer);
        return true;
    }
    return false;
}

/**
 * Attempt to find the best renderer to use, if one is not selected.
 * We just assume the first available renderer is the best one :)
 * @return true if renderer was found, false if not.
 */
static bool find_best_renderer(void) {
    if(renderer_count > 0) {
        available_renderers[0].set_callbacks(&current_renderer);
        return true;
    }
    return false;
}

static bool video_find_renderer(const char *try_name) {
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
    current_renderer.create(&current_renderer);
    if(!current_renderer.setup_context(current_renderer.ctx, window_w, window_h, fullscreen, vsync)) {
        goto exit_1;
    }
    return true;

exit_1:
    current_renderer.destroy(&current_renderer);
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
    current_renderer.destroy(&current_renderer);
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

void video_render_text_block(text_object *text) {
    for(uint32_t i = 0; i < text->letter_count; i += 1) {
        const letter *letterptr = &(text->letters[i]);
        video_draw_offset(letterptr->sur, letterptr->x, letterptr->y, letterptr->offset, letterptr->limit);
    }
}
