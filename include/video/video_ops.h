#ifndef _VIDEO_OPS_H
#define _VIDEO_OPS_H

#include "video/color.h"
#include "video/surface.h"
#include <SDL2/SDL.h>

typedef struct video_state_t video_state;

typedef void (*render_close_cb)(video_state *state);
typedef void (*render_reinit_cb)(video_state *state);
typedef void (*render_prepare_cb)(video_state *state);
typedef void (*render_finish_cb)(video_state *state);

typedef void (*render_background_cb)(
                    video_state *state,
                    surface *sur);

typedef void (*render_sprite_fsot_cb)(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst,
                    SDL_BlendMode blend_mode, 
                    int pal_offset, 
                    SDL_RendererFlip flip_mode, 
                    uint8_t opacity,
                    color tint);

typedef struct video_render_cbs_t {
    render_close_cb render_close;
    render_reinit_cb render_reinit;
    render_prepare_cb render_prepare;
    render_finish_cb render_finish;
    render_sprite_fsot_cb render_fsot;
    render_background_cb render_background;
} video_render_cbs;

#endif // _VIDEO_OPS_H
