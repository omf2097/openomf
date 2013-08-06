#ifndef _VIDEO_H
#define _VIDEO_H

#include "video/color.h"

#define NATIVE_W 320
#define NATIVE_H 200

#define BLEND_ADDITIVE 0
#define BLEND_ALPHA 1
#define BLEND_ALPHA_FULL 2

#define FLIP_NONE 0
#define FLIP_HORIZONTAL 1
#define FLIP_VERTICAL 2

int video_init(int window_w, int window_h, int fullscreen, int vsync); // Create window etc.
int video_reinit(int window_w, int window_h, int fullscreen, int vsync);
void video_render_prepare();
void video_render_sprite(texture *texture, int x, int y, unsigned int render_mode);
void video_render_sprite_flip(texture *texture, int x, int y, unsigned int render_mode, unsigned int flip_mode);
void video_render_colored_quad(int x, int y, int w, int h, color c);
void video_render_char(texture *texture, int x, int y, color c);
void video_render_finish();
void video_render_background(texture *tex);
void video_close();
void video_screenshot(image *img);

#endif // _VIDEO_H
