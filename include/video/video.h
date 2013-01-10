#ifndef _VIDEO_H
#define _VIDEO_H

#define NATIVE_W 320
#define NATIVE_H 200

#define RENDER_BLEND 0
#define RENDER_ADDITIVE 1

#include "video/texture.h"

typedef struct gl_sprite_t {
    texture tex;
    unsigned int x,y;
    unsigned int rendering_mode;
} gl_sprite;

int video_init(int window_w, int window_h, int fullscreen, int vsync); // Create window etc.
void video_render_prepare();
void video_render_sprite(gl_sprite *sprite);
void video_render_finish();
void video_close();

#endif // _VIDEO_H