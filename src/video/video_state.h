#ifndef VIDEO_STATE_H
#define VIDEO_STATE_H

#include <stdbool.h>

#include "formats/palette.h"
#include "video/screen_palette.h"
#include <SDL.h>

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

    bool render_bg_separately;
    SDL_Texture *fg_target;
    SDL_Texture *bg_target;

    // Palettes
    palette *base_palette;          // Copy of the scenes base palette
    screen_palette *screen_palette; // Normal rendering palette
    screen_palette *extra_palette;  // Reflects base palette, used for additive blending
} video_state;

#endif // VIDEO_STATE_H
