#include "video/video_hw.h"
#include "video/tcache.h"
#include "utils/log.h"

void hw_render_close(video_state *state) {

}

void hw_render_reinit(video_state *state) {

}

void hw_render_prepare(video_state *state) {

}

void hw_render_finish(video_state *state) {

}

void hw_scale_rect(video_state *state, SDL_Rect *rct) {
    rct->w = rct->w * state->scale_factor;
    rct->h = rct->h * state->scale_factor;
    rct->x = rct->x * state->scale_factor;
    rct->y = rct->y * state->scale_factor;
}

void hw_render_background(
                    video_state *state,
                    surface *sur) {

    SDL_Texture *tex = tcache_get(sur, state->cur_palette, NULL, 0);
    if(tex == NULL)
        return;
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_NONE);
    SDL_RenderCopy(state->renderer, tex, NULL, NULL);
}

void hw_render_sprite_fsot(
                    video_state *state,
                    surface *sur,
                    SDL_Rect *dst,
                    SDL_BlendMode blend_mode,
                    int pal_offset,
                    SDL_RendererFlip flip_mode,
                    uint8_t opacity,
                    color color_mod) {

    hw_scale_rect(state, dst);
    SDL_Texture *tex = tcache_get(sur, state->cur_palette, NULL, pal_offset);
    if(tex == NULL)
        return;
    SDL_SetTextureAlphaMod(tex, opacity);
    SDL_SetTextureColorMod(tex, color_mod.r, color_mod.g, color_mod.b);
    SDL_SetTextureBlendMode(tex, blend_mode);
    SDL_RenderCopyEx(state->renderer, tex, NULL, dst, 0, NULL, flip_mode);
}


void video_hw_init(video_state *state) {
    state->cb.render_close = hw_render_close;
    state->cb.render_reinit = hw_render_reinit;
    state->cb.render_prepare = hw_render_prepare;
    state->cb.render_finish = hw_render_finish;
    state->cb.render_fsot = hw_render_sprite_fsot;
    state->cb.render_background = hw_render_background;
    DEBUG("Switched to hardware renderer.");
}
