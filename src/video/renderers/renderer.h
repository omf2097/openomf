#ifndef RENDERER_H
#define RENDERER_H

#include "video/surface.h"

typedef struct renderer renderer;

// Asynchronous screenshot signal, renderer must call this when it has the screenshot data.
typedef void (*video_screenshot_signal)(const SDL_Rect *rect, unsigned char *data, bool flipped);

// Metadata functions, all must be implemented. These must NOT require context or renderer state to be initialized!
typedef bool (*is_available_fn)(void);
typedef const char *(*get_description_fn)(void);
typedef const char *(*get_name_fn)(void);

// These initialize the renderer itself; they should be used to reserve and free internal context objects.
typedef void (*init_renderer_fn)(renderer *renderer);
typedef void (*close_renderer_fn)(renderer *renderer);

// Renderer initialization and de-initialization, these must be implemented.
typedef bool (*setup_context_fn)(void *ctx, int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*get_context_state_fn)(void *ctx, int *window_w, int *window_h, bool *fullscreen, bool *vsync);
typedef bool (*reset_context_with_fn)(void *ctx, int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*reset_context_fn)(void *ctx);
typedef void (*close_context_fn)(void *ctx);

// Rendering functions, there must be implemented
typedef void (*draw_surface_fn)(void *ctx, const surface *src_surface, SDL_Rect *rect, int remap_offset,
                                int remap_rounds, int palette_offset, int palette_limit, int opacity,
                                unsigned int flip_mode, unsigned int options);
typedef void (*move_target_fn)(void *ctx, int x, int y);

// Onscreen rendering state management, these must be implemented
typedef void (*render_prepare_fn)(void *ctx);
typedef void (*render_finish_fn)(void *ctx);

// Offscreen rendering state management, these must be implemented
typedef void (*render_area_prepare_fn)(void *ctx, const SDL_Rect *area);
typedef void (*render_area_finish_fn)(void *ctx, surface *dst);

// Screenshotting, this /should/ be implemented (but is not required).
typedef void (*capture_screen_fn)(void *ctx, video_screenshot_signal screenshot_cb);

// Extra signals, implemented only if renderer implementation supports and/or requires it
typedef void (*signal_scene_change_fn)(void *ctx);
typedef void (*signal_draw_atlas_fn)(void *ctx, bool toggle);

struct renderer {
    is_available_fn is_available;
    get_description_fn get_description;
    get_name_fn get_name;

    init_renderer_fn init;
    close_renderer_fn close;

    setup_context_fn setup_context;
    reset_context_with_fn reset_context_with;
    reset_context_fn reset_context;
    get_context_state_fn get_context_state;
    close_context_fn close_context;

    draw_surface_fn draw_surface;
    move_target_fn move_target;

    render_prepare_fn render_prepare;
    render_finish_fn render_finish;
    render_area_prepare_fn render_area_prepare;
    render_area_finish_fn render_area_finish;

    capture_screen_fn capture_screen;

    signal_scene_change_fn signal_scene_change;
    signal_draw_atlas_fn signal_draw_atlas;

    void *ctx;
};

#endif // RENDERER_H
