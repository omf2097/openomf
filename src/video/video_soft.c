#include <stdlib.h>
#include "video/video_soft.h"
#include "utils/log.h"

typedef struct soft_renderer_t {
    char tmp[320*200*4];
    surface lower;
    SDL_Surface *higher;
} soft_renderer;

SDL_Surface* surface_from_pixels(char *pixels, int w, int h) {
    return SDL_CreateRGBSurfaceFrom(pixels, 
                                    w, h, 
                                    32, 
                                    w*4,
                                    0x000000FF,
                                    0x0000FF00,
                                    0x00FF0000,
                                    0xFF000000);
}


void soft_render_close(video_state *state) {
    soft_renderer *sr = state->userdata;
    SDL_FreeSurface(sr->higher);
    surface_free(&sr->lower);
    free(sr);
}

void soft_render_reinit(video_state *state) {

}

void soft_render_prepare(video_state *state) {
    soft_renderer *sr = state->userdata;
    SDL_FillRect(sr->higher, NULL, SDL_MapRGBA(sr->higher->format, 0, 0, 0, 0));
}

void soft_render_finish(video_state *state) {
    soft_renderer *sr = state->userdata;
    SDL_Texture *tex;

    // Blit lower
    surface_to_rgba(&sr->lower, sr->tmp, state->cur_palette, NULL, 0);
    SDL_Surface *low_s = surface_from_pixels(sr->tmp, 320, 200);
    tex = SDL_CreateTextureFromSurface(state->renderer, low_s);
    SDL_RenderCopy(state->renderer, tex, NULL, NULL);
    SDL_DestroyTexture(tex);

    // Blit upper
    tex = SDL_CreateTextureFromSurface(state->renderer, sr->higher);
    SDL_RenderCopy(state->renderer, tex, NULL, NULL);
    SDL_DestroyTexture(tex);
}

void soft_render_background(
                    video_state *state,
                    surface *sur) {

    soft_renderer *sr = state->userdata;
    surface_alpha_blit(&sr->lower, sur, 0, 0, 0);
}

void soft_render_sprite_fso(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst,
                    SDL_BlendMode blend_mode, 
                    int pal_offset, 
                    SDL_RendererFlip flip_mode, 
                    uint8_t opacity) {

    // No scaling,
    // No opacity for paletted surfaces

    soft_renderer *sr = state->userdata;
    if(sur->type == SURFACE_TYPE_PALETTE) {
        if(blend_mode == SDL_BLENDMODE_ADD) {
            surface_additive_blit(&sr->lower, sur, dst->x, dst->y, state->base_palette, flip_mode);
        } else {
            surface_alpha_blit(&sr->lower, sur, dst->x, dst->y, flip_mode);
        }
    } else {
        surface_to_rgba(sur, sr->tmp, state->cur_palette, NULL, 0);
        SDL_Surface *s = surface_from_pixels(sr->tmp, sur->w, sur->h);
        SDL_SetSurfaceAlphaMod(s, opacity);
        SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
        SDL_BlitSurface(s, NULL, sr->higher, dst);
    }
}

void soft_render_sprite_tint(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst, 
                    color c, 
                    int pal_offset) {

    soft_renderer *sr = state->userdata;
    surface_to_rgba(sur, sr->tmp, state->cur_palette, NULL, 0);
    SDL_Surface *s = surface_from_pixels(sr->tmp, sur->w, sur->h);

    SDL_SetSurfaceColorMod(s, c.r, c.g, c.b);
    SDL_SetSurfaceAlphaMod(s, 0xFF);
    SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);

    SDL_BlitSurface(sr->higher, NULL, s, dst);
}

void soft_render_sprite_shadow(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst, 
                    int pal_offset,
                    SDL_RendererFlip flip_mode) {

    return; // NO-OP for now
}

void video_soft_init(video_state *state) {
    soft_renderer *sr = malloc(sizeof(soft_renderer));
    sr->higher = SDL_CreateRGBSurface(0, 
                                    320,
                                    200,
                                    32,
                                    0x000000FF,
                                    0x0000FF00,
                                    0x00FF0000,
                                    0xFF000000);
    SDL_SetSurfaceRLE(sr->higher, 1);
    surface_create(&sr->lower, SURFACE_TYPE_PALETTE, 320, 200);
    state->userdata = sr;

    state->cb.render_close = soft_render_close;
    state->cb.render_reinit = soft_render_reinit;
    state->cb.render_prepare = soft_render_prepare;
    state->cb.render_finish = soft_render_finish;
    state->cb.render_fso = soft_render_sprite_fso;
    state->cb.render_tint = soft_render_sprite_tint;
    state->cb.render_shadow = soft_render_sprite_shadow;
    state->cb.render_background = soft_render_background;
    DEBUG("Switched to software renderer.");
}
