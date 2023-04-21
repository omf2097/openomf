#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

#include "formats/palette.h"
#include "video/color.h"
#include "video/image.h"
#include "video/screen_palette.h"
#include "video/surface.h"

#define NATIVE_W 320
#define NATIVE_H 200

enum VIDEO_BLEND_MODE
{
    BLEND_ADDITIVE = 0,
    BLEND_ALPHA
};

enum VIDEO_FLIP_MODE
{
    FLIP_NONE = 0,
    FLIP_HORIZONTAL = 0x1,
    FLIP_VERTICAL = 0x2,
};

int video_init(int window_w, int window_h, bool fullscreen, bool vsync);
int video_reinit(int window_w, int window_h, bool fullscreen, bool vsync);
void video_reinit_renderer(void);
void video_get_state(int *w, int *h, int *fs, int *vsync);
void video_move_target(int x, int y);

void video_render_sprite(surface *sur, int x, int y, unsigned int render_mode, int pal_offset);

void video_render_sprite_size(surface *sur, int sx, int sy, int sw, int sh);

void video_render_sprite_tint(surface *sur, int x, int y, color c, int pal_offset);

void video_render_sprite_flip_scale_opacity(surface *sur, int x, int y, unsigned int render_mode, int pal_offset,
                                            unsigned int flip_mode, float x_percent, float y_percent, uint8_t opacity);

void video_render_sprite_flip_scale_opacity_tint(surface *sur, int x, int y, unsigned int render_mode, int pal_offset,
                                                 unsigned int flip_mode, float x_percent, float y_percent,
                                                 uint8_t opacity, color tint);

void video_tick(void);
void video_render_background(surface *sur);
void video_render_prepare(void);
void video_render_finish(void);
void video_close(void);
int video_screenshot(image *img);
int video_area_capture(surface *sur, int x, int y, int w, int h);
void video_set_fade(float fade);
void video_render_bg_separately(bool separate);

void video_set_base_palette(const palette *src);
palette *video_get_base_palette(void);
void video_force_pal_refresh(void);
void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount);
void video_copy_base_pal_range(const palette *src, int src_start, int dst_start, int amount);
screen_palette *video_get_pal_ref(void);

#endif // VIDEO_H
