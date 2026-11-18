#include "video/renderers/software/software_renderer.h"

#include "platform/time.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/enums.h"
#include "video/framebuffer/presenter.h"
#include "video/software/compositor.h"
#include "video/vga_state.h"
#include "video/video.h"

#include <string.h>

typedef struct software_context {
    software_compositor compositor;
    framebuffer_presenter presenter;
    int screen_w;
    int screen_h;
    int aspect;
    int fb_scale;
    int scaling_mode;
    int framerate_limit;
    int target_x;
    int target_y;
    uint64_t last_frame;
    irect offscreen_area;
    video_screenshot_signal screenshot;
    bool fullscreen;
    bool vsync;
} software_context;

static bool available(void) {
    return framebuffer_presenter_count() > 0;
}

static const char *get_name(void) {
    return "Software";
}

static const char *get_description(void) {
    return "Portable indexed CPU compositor";
}

static void create(renderer *r) {
    r->ctx = omf_calloc(1, sizeof(software_context));
}

static void destroy(renderer *r) {
    software_context *ctx = r->ctx;
    if(ctx) {
        framebuffer_presenter_destroy(&ctx->presenter);
        software_compositor_close(&ctx->compositor);
        omf_free(ctx);
    }
    r->ctx = NULL;
}

static bool setup_context(void *userdata, int width, int height, bool fullscreen, bool vsync, int aspect,
                          int framerate_limit, int fb_scale, int scaling_mode) {
    software_context *ctx = userdata;
    if(!ctx || !software_compositor_init(&ctx->compositor)) {
        return false;
    }
    if(!framebuffer_presenter_create(&ctx->presenter, NULL) ||
       !ctx->presenter.open(ctx->presenter.ctx, width, height, fullscreen, vsync, scaling_mode)) {
        framebuffer_presenter_destroy(&ctx->presenter);
        software_compositor_close(&ctx->compositor);
        log_error("Software framebuffer presenter could not initialize");
        return false;
    }
    ctx->screen_w = width;
    ctx->screen_h = height;
    ctx->fullscreen = fullscreen;
    ctx->vsync = vsync;
    ctx->aspect = aspect;
    ctx->fb_scale = fb_scale;
    ctx->scaling_mode = scaling_mode;
    ctx->framerate_limit = framerate_limit;
    ctx->last_frame = time_ms();
    vga_state_mark_dirty();
    log_info("Software renderer using %s framebuffer presenter", ctx->presenter.get_name());
    return true;
}

static void get_context_state(void *userdata, int *width, int *height, bool *fullscreen, bool *vsync, int *aspect,
                              int *fb_scale) {
    software_context *ctx = userdata;
    if(width) {
        *width = ctx->screen_w;
    }
    if(height) {
        *height = ctx->screen_h;
    }
    if(fullscreen) {
        *fullscreen = ctx->fullscreen;
    }
    if(vsync) {
        *vsync = ctx->vsync;
    }
    if(aspect) {
        *aspect = ctx->aspect;
    }
    if(fb_scale) {
        *fb_scale = ctx->fb_scale;
    }
}

static bool reset_context_with(void *userdata, int width, int height, bool fullscreen, bool vsync, int aspect,
                               int framerate_limit, int fb_scale, int scaling_mode) {
    software_context *ctx = userdata;
    ctx->presenter.close(ctx->presenter.ctx);
    if(!ctx->presenter.open(ctx->presenter.ctx, width, height, fullscreen, vsync, scaling_mode)) {
        return false;
    }
    ctx->screen_w = width;
    ctx->screen_h = height;
    ctx->fullscreen = fullscreen;
    ctx->vsync = vsync;
    ctx->aspect = aspect;
    ctx->framerate_limit = framerate_limit;
    ctx->fb_scale = fb_scale;
    ctx->scaling_mode = scaling_mode;
    ctx->last_frame = time_ms();
    return true;
}

static void reset_context(void *userdata) {
    software_context *ctx = userdata;
    reset_context_with(ctx, ctx->screen_w, ctx->screen_h, ctx->fullscreen, ctx->vsync, ctx->aspect,
                       ctx->framerate_limit, ctx->fb_scale, ctx->scaling_mode);
}

static void close_context(void *userdata) {
    software_context *ctx = userdata;
    ctx->presenter.close(ctx->presenter.ctx);
}

static void sync_vga_state(software_context *ctx) {
    vga_palette *palette;
    vga_remap_tables *remaps;
    if(vga_state_is_palette_dirty(&palette, NULL, NULL)) {
        software_compositor_set_palette(&ctx->compositor, palette);
        vga_state_mark_palette_flushed();
    }
    if(vga_state_is_remap_dirty(&remaps)) {
        software_compositor_set_remaps(&ctx->compositor, remaps);
        vga_state_mark_remaps_flushed();
    }
}

static void draw_surface(void *userdata, const surface *src, irect *rect, int remap_offset, int remap_rounds,
                         int palette_offset, int palette_limit, int opacity, unsigned int flip, unsigned int options) {
    software_context *ctx = userdata;
    if(options & SPRITE_REMAP) {
        sync_vga_state(ctx);
    }
    software_compositor_draw(&ctx->compositor, src, rect, remap_offset, remap_rounds, palette_offset, palette_limit,
                             opacity, flip, options);
}

static void move_target(void *userdata, int x, int y) {
    software_context *ctx = userdata;
    ctx->target_x = x;
    ctx->target_y = y;
}

static void render_prepare(void *userdata, unsigned framebuffer_options) {
    software_context *ctx = userdata;
    software_compositor_prepare(&ctx->compositor, framebuffer_options);
}

static void capture_screenshot(software_context *ctx, const uint32_t *rgba) {
    if(!ctx->screenshot) {
        return;
    }
    unsigned char *rgb = omf_malloc((size_t)NATIVE_W * NATIVE_H * 3);
    if(!rgb) {
        return;
    }
    for(int i = 0; i < NATIVE_W * NATIVE_H; i++) {
        rgb[3 * i] = (uint8_t)rgba[i];
        rgb[3 * i + 1] = (uint8_t)(rgba[i] >> 8);
        rgb[3 * i + 2] = (uint8_t)(rgba[i] >> 16);
    }
    const irect area = irect_create(0, 0, NATIVE_W, NATIVE_H);
    ctx->screenshot(&area, rgb, false);
    omf_free(rgb);
    ctx->screenshot = NULL;
}

static void render_finish(void *userdata) {
    software_context *ctx = userdata;
    sync_vga_state(ctx);
    const uint32_t *pixels = software_compositor_resolve(&ctx->compositor);
    capture_screenshot(ctx, pixels);

    irect viewport = irect_create(0, 0, ctx->screen_w, ctx->screen_h);
    if(ctx->aspect == 0) {
        const int fit_w = ctx->screen_h * 4 / 3;
        if(fit_w <= ctx->screen_w) {
            viewport.x = (ctx->screen_w - fit_w) / 2;
            viewport.w = fit_w;
        } else {
            viewport.h = ctx->screen_w * 3 / 4;
            viewport.y = (ctx->screen_h - viewport.h) / 2;
        }
    }
    viewport.x += ctx->target_x * viewport.w / NATIVE_W;
    viewport.y += ctx->target_y * viewport.w / NATIVE_W;

    const framebuffer_image image = {pixels, NATIVE_W, NATIVE_H, NATIVE_W * sizeof(*pixels),
                                     FRAMEBUFFER_FORMAT_ABGR8888};
    if(!ctx->presenter.present(ctx->presenter.ctx, &image, &viewport)) {
        log_error("Software framebuffer presentation failed");
    }
    if(ctx->framerate_limit > 0) {
        const uint64_t min_frame = 1000u / (unsigned)ctx->framerate_limit;
        const uint64_t elapsed = time_ms() - ctx->last_frame;
        if(elapsed < min_frame) {
            time_sleep_ms((uint32_t)(min_frame - elapsed));
        }
    }
    ctx->last_frame = time_ms();
}

static void render_area_prepare(void *userdata, const irect *area) {
    software_context *ctx = userdata;
    ctx->offscreen_area = *area;
    software_compositor_prepare(&ctx->compositor, 0);
}

static void render_area_finish(void *userdata, surface *dst) {
    software_context *ctx = userdata;
    software_compositor_capture_area(&ctx->compositor, &ctx->offscreen_area, dst);
}

static void capture_screen(void *userdata, video_screenshot_signal callback) {
    software_context *ctx = userdata;
    ctx->screenshot = callback;
}

static void signal_scene_change(void *userdata) {
    (void)userdata;
    vga_state_mark_dirty();
}

static void signal_draw_atlas(void *userdata, bool toggle) {
    (void)userdata;
    (void)toggle;
}

void software_renderer_set_callbacks(renderer *r) {
    r->is_available = available;
    r->get_name = get_name;
    r->get_description = get_description;
    r->create = create;
    r->destroy = destroy;
    r->setup_context = setup_context;
    r->get_context_state = get_context_state;
    r->reset_context_with = reset_context_with;
    r->reset_context = reset_context;
    r->close_context = close_context;
    r->draw_surface = draw_surface;
    r->move_target = move_target;
    r->render_prepare = render_prepare;
    r->render_finish = render_finish;
    r->render_area_prepare = render_area_prepare;
    r->render_area_finish = render_area_finish;
    r->capture_screen = capture_screen;
    r->signal_scene_change = signal_scene_change;
    r->signal_draw_atlas = signal_draw_atlas;
}
