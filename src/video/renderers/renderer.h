#ifndef RENDERER_H
#define RENDERER_H

#include "video/surface.h"

// Callback from renderer
typedef void (*video_screenshot_signal)(const SDL_Rect *rect, unsigned char *data,
                                        bool flipped); // Asynchronous screenshot signal

// Metadata functions, all must be implemented
typedef bool (*is_available_fn)(void);
typedef const char *(*get_description_fn)(void);
typedef const char *(*get_name_fn)(void);

// Renderer functions that must be implemented
typedef void (*setup_context_fn)(int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*get_context_state_fn)(int *window_w, int *window_h, bool *fullscreen, bool *vsync);
typedef void (*reset_context_with_fn)(int window_w, int window_h, bool fullscreen, bool vsync);
typedef void (*reset_context_fn)(void);
typedef void (*close_context_fn)(void);
typedef void (*draw_surface_fn)(const surface *src_surface, int x, int y, int w, int h, int remap_offset,
                                int remap_rounds, int palette_offset, int palette_limit, int opacity,
                                unsigned int flip_mode, unsigned int options);
typedef void (*move_target_fn)(int x, int y);

// Renderer functions that should be implemented
typedef void (*screenshot_fn)(video_screenshot_signal screenshot_cb);

// Extra signals, implemented only if renderer implementation supports and/or requires it
typedef void (*signal_scene_change_fn)(void);
typedef void (*signal_draw_atlas_fn)(bool toggle);

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

    screenshot_fn screenshot;

    signal_scene_change_fn signal_scene_change;
    signal_draw_atlas_fn signal_draw_atlas;

    void *renderer_data;
} renderer;

#endif // RENDERER_H
