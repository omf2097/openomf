#ifndef _VIDEO_H
#define _VIDEO_H

#include "video/color.h"
#include "video/surface.h"
#include "video/image.h"
#include "video/screen_palette.h"
#include "resources/palette.h"

#define NATIVE_W 320
#define NATIVE_H 200

enum VIDEO_BLEND_MODE {
    BLEND_ADDITIVE = 0,
    BLEND_ALPHA
};

enum VIDEO_FLIP_MODE {
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
};

enum VIDEO_RENDERER {
    VIDEO_RENDERER_QUIRKS = 0,
    VIDEO_RENDERER_HW,
};

int video_init(int window_w,
               int window_h,
               int fullscreen,
               int vsync,
               const char* scaler_name,
               int scale_factor);
int video_reinit(int window_w,
                 int window_h,
                 int fullscreen,
                 int vsync,
                 const char* scaler_name,
                 int scale_factor);
void video_reinit_renderer();
void video_get_state(int *w, int *h, int *fs, int *vsync);
void video_move_target(int x, int y);

void video_render_sprite(
    surface *sur,
    int x,
    int y,
    unsigned int render_mode,
    int pal_offset);

void video_render_sprite_flip_scale(
    surface *sur,
    int x,
    int y,
    unsigned int render_mode,
    int pal_offset,
    unsigned int flip_mode,
    float y_percent);

void video_render_sprite_tint(
    surface *sur,
    int x,
    int y,
    color c,
    int pal_offset);

void video_render_sprite_flip_scale_opacity(
    surface *sur,
    int x,
    int y,
    unsigned int render_mode,
    int pal_offset,
    unsigned int flip_mode,
    float y_percent,
    uint8_t opacity);

void video_render_sprite_flip_scale_opacity_tint(
    surface *sur,
    int x,
    int y,
    unsigned int render_mode,
    int pal_offset,
    unsigned int flip_mode,
    float y_percent,
    uint8_t opacity,
    color tint);

void video_select_renderer(int renderer);
void video_tick();
void video_render_background(surface *sur);
void video_render_prepare();
void video_render_finish();
void video_close();
void video_screenshot(image *img);
void video_set_fade(float fade);

void video_set_base_palette(const palette *src);
palette *video_get_base_palette();
void video_force_pal_refresh();
void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount);
screen_palette* video_get_pal_ref();

#endif // _VIDEO_H
