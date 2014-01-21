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

void hw_render_background(
                    video_state *state,
                    surface *sur) {

    SDL_Texture *tex = tcache_get(sur, state->renderer, state->cur_palette, NULL, 0);
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(state->renderer, tex, NULL, NULL);
}

void hw_render_sprite_fso(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst,
                    SDL_BlendMode blend_mode, 
                    int pal_offset, 
                    SDL_RendererFlip flip_mode, 
                    uint8_t opacity) {

    SDL_Texture *tex = tcache_get(sur, state->renderer, state->cur_palette, NULL, pal_offset);
    SDL_SetTextureAlphaMod(tex, opacity);
    SDL_SetTextureColorMod(tex, 0xFF, 0xFF, 0xFF);
    SDL_SetTextureBlendMode(tex, blend_mode);
    SDL_RenderCopyEx(state->renderer, tex, NULL, dst, 0, NULL, flip_mode);
}

void hw_render_sprite_tint(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst, 
                    color c, 
                    int pal_offset) {

    SDL_Texture *tex = tcache_get(sur, state->renderer, state->cur_palette, NULL, pal_offset);
    SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(tex, 0xFF);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyEx(state->renderer, tex, NULL, dst, 0, NULL, 0);
}

void hw_render_sprite_shadow(
                    video_state *state,
                    surface *sur, 
                    SDL_Rect *dst, 
                    int pal_offset,
                    SDL_RendererFlip flip_mode) {

    SDL_Texture *tex = tcache_get(sur, state->renderer, state->cur_palette, NULL, pal_offset);
    SDL_SetTextureColorMod(tex, 0, 0, 0);
    SDL_SetTextureAlphaMod(tex, 96);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_RenderCopyEx(state->renderer, tex, NULL, dst, 0, NULL, flip_mode);
}

void video_hw_init(video_state *state) {
    state->cb.render_close = hw_render_close;
    state->cb.render_reinit = hw_render_reinit;
    state->cb.render_prepare = hw_render_prepare;
    state->cb.render_finish = hw_render_finish;
    state->cb.render_fso = hw_render_sprite_fso;
    state->cb.render_tint = hw_render_sprite_tint;
    state->cb.render_shadow = hw_render_sprite_shadow;
    state->cb.render_background = hw_render_background;
    DEBUG("Switched to hardware renderer.");
}
