#ifndef _VIDEO_STATE_H
#define _VIDEO_STATE_H

#include <SDL2/SDL.h>
#include "resources/palette.h"
#include "video/screen_palette.h" 
#include "video/video_ops.h"

typedef struct video_state_t {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int w;
    int h;
    int fs;
    int vsync;

    float fade;
    int target_move_x;
    int target_move_y;

    int cur_renderer;
    SDL_Texture *target;

    // Palettes
    palette *base_palette;
    screen_palette *cur_palette;

    // Renderer
    video_render_cbs cb;
    void *userdata;
} video_state;

#endif // _VIDEO_STATE_H
