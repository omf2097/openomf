#ifndef RENDERER_H
#define RENDERER_H

#include "video/surface.h"

// Asynchronous screenshot signal
typedef void (*video_screenshot_signal)(const SDL_Rect *rect, unsigned char *data, bool flipped);

// Metadata functions, all must be implemented
typedef bool (*is_available_fn)(void *ctx);
typedef const char *(*get_description_fn)(void);
typedef const char *(*get_name_fn)(void);

// Renderer functions that must be implemented
typedef bool (*setup_context_fn)(void *ctx, int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*get_context_state_fn)(void *ctx, int *window_w, int *window_h, bool *fullscreen, bool *vsync);
typedef bool (*reset_context_with_fn)(void *ctx, int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*reset_context_fn)(void *ctx);
typedef void (*close_context_fn)(void *ctx);

typedef void (*draw_surface_fn)(void *ctx, const surface *src_surface, SDL_Rect *rect, int remap_offset,
                                int remap_rounds, int palette_offset, int palette_limit, int opacity,
                                unsigned int flip_mode, unsigned int options);
typedef void (*move_target_fn)(void *ctx, int x, int y);

typedef void (*render_prepare_fn)(void *ctx);
typedef void (*render_finish_fn)(void *ctx);

// Renderer functions that should be implemented
typedef void (*capture_screen_fn)(void *ctx, video_screenshot_signal screenshot_cb);
typedef void (*capture_area_fn)(void *ctx, surface *dst, int x, int y, int w, int h);

// Extra signals, implemented only if renderer implementation supports and/or requires it
typedef void (*signal_scene_change_fn)(void *ctx);
typedef void (*signal_draw_atlas_fn)(void *ctx, bool toggle);

typedef struct renderer {
    is_available_fn is_available;
    get_description_fn get_description;
    get_name_fn get_name;

    setup_context_fn setup_context;
    reset_context_with_fn reset_context_with;
    reset_context_fn reset_context;
    get_context_state_fn get_context_state;
    close_context_fn close_context;

    draw_surface_fn draw_surface;
    move_target_fn move_target;

    render_prepare_fn render_prepare;
    render_finish_fn render_finish;

    capture_screen_fn capture_screen;
    capture_area_fn capture_area;

    signal_scene_change_fn signal_scene_change;
    signal_draw_atlas_fn signal_draw_atlas;

    void *ctx;
} renderer;

#endif // RENDERER_H
