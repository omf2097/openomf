#ifndef _VIDEO_H
#define _VIDEO_H

#include "video/color.h"
#include "video/surface.h"
#include "video/image.h"

#define NATIVE_W 320
#define NATIVE_H 200

#define BLEND_ADDITIVE 0
#define BLEND_ALPHA 1
#define BLEND_ALPHA_FULL 2
#define BLEND_ALPHA_CONSTANT 3

#define FLIP_NONE 0
#define FLIP_HORIZONTAL 1
#define FLIP_VERTICAL 2

int video_init(int window_w, int window_h, int fullscreen, int vsync); // Create window etc.
int video_reinit(int window_w, int window_h, int fullscreen, int vsync);
void video_render_prepare();
void video_render_sprite(surface *sur, int x, int y, unsigned int render_mode);
void video_render_sprite_flip_scale(surface *sur, int x, int y, unsigned int render_mode, unsigned int flip_mode, float y_percent);
#define video_render_sprite_flip(tex, sx, sy, render_mode, flip_mode) video_render_sprite_flip_scale(tex, sx, sy, render_mode, flip_mode, 1.0)
void video_render_char(surface *sur, int x, int y, color c);
void video_render_finish();
void video_render_background(surface *sur);
void video_close();
void video_screenshot(image *img);

void video_set_base_palette(const palette *src);
void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount);

#endif // _VIDEO_H
