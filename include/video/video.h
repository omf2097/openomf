#ifndef _VIDEO_H
#define _VIDEO_H

#define NATIVE_W 320
#define NATIVE_H 200

#define BLEND_ADDITIVE 0
#define BLEND_ALPHA 1

typedef struct texture_t texture;

int video_init(int window_w, int window_h, int fullscreen, int vsync); // Create window etc.
void video_render_prepare();
void video_queue_add(texture *texture, unsigned int x, unsigned int y, unsigned int render_mode, int priority);
void video_queue_remove(texture *tex);
void video_render_finish();
void video_set_background(texture *tex);
void video_close();

#endif // _VIDEO_H