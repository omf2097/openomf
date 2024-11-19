#include "video/renderers/null/null_renderer.h"

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/vga_state.h"

static bool is_available(void) {
    return true;
}

static const char *get_description(void) {
    return "Zero output renderer";
}

static const char *get_name(void) {
    return "NULL";
}

static bool setup_context(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync) {
    INFO("NULL Renderer initialized!");
    return true;
}

static void get_context_state(void *userdata, int *window_w, int *window_h, bool *fullscreen, bool *vsync) {
    if(window_w != NULL)
        *window_w = 0;
    if(window_h != NULL)
        *window_h = 0;
    if(fullscreen != NULL)
        *fullscreen = false;
    if(vsync != NULL)
        *vsync = false;
}

static bool reset_context_with(void *userdata, int window_w, int window_h, bool fullscreen, bool vsync) {
    INFO("NULL renderer reset.");
    return true;
}
static void reset_context(void *userdata) {
}
static void close_context(void *userdata) {
    INFO("NULL renderer closed.");
}

static void draw_surface(void *userdata, const surface *src_surface, SDL_Rect *dst, int remap_offset, int remap_rounds,
                         int palette_offset, int palette_limit, int opacity, unsigned int flip_mode,
                         unsigned int options) {
}
static void move_target(void *userdata, int x, int y) {
}
static void render_prepare(void *userdata) {
}

static void render_finish(void *userdata) {
}
static void render_area_prepare(void *userdata, const SDL_Rect *area) {
}
static void render_area_finish(void *userdata, surface *dst) {
}

static void capture_screen(void *userdata, video_screenshot_signal screenshot_cb) {
}

static void signal_scene_change(void *userdata) {
}
static void signal_draw_atlas(void *userdata, bool toggle) {
}

static void renderer_create(renderer *gl3_renderer) {
}
static void renderer_destroy(renderer *gl3_renderer) {
}

void null_renderer_set_callbacks(renderer *gl3_renderer) {
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
