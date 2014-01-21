#include "video/video.h"
#include "video/image.h"
#include "video/tcache.h"
#include "utils/log.h"
#include "utils/list.h"
#include "resources/palette.h"
#include "video/video_state.h"
#include "video/video_hw.h"
#include "video/video_soft.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

static video_state state;

void reset_targets() {
    if(state.target != NULL) {
        SDL_DestroyTexture(state.target);
    }
    state.target = SDL_CreateTexture(state.renderer,
                                     SDL_PIXELFORMAT_ABGR8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     NATIVE_W, NATIVE_H);
}

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    state.w = window_w;
    state.h = window_h;
    state.fs = fullscreen;
    state.vsync = vsync;
    state.fade = 1.0f;
    state.target = NULL;

    // Clear palettes
    state.cur_palette = malloc(sizeof(screen_palette));
    state.base_palette = malloc(sizeof(palette));
    state.cur_palette->version = 1;
    memset(state.cur_palette->data, 0, 768);

    // Open window
    state.window = SDL_CreateWindow(
        "OpenOMF",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_w,
        window_h,
        SDL_WINDOW_SHOWN);
    if(state.window == NULL) {
        PERROR("Could not create window: %s", SDL_GetError());
        return 1;
    }

    // Form flags
    int renderer_flags = SDL_RENDERER_ACCELERATED;
    if(state.vsync) {
        renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    // Create renderer
    state.renderer = SDL_CreateRenderer(
        state.window, 
        -1, 
        renderer_flags);
    if(state.renderer == NULL) {
        PERROR("Could not create renderer: %s", SDL_GetError());
        return 1;
    }

    // Default resolution for renderer. This will them get scaled up to screen size.
    SDL_RenderSetLogicalSize(state.renderer, NATIVE_W, NATIVE_H);

    // Disable screensaver :/
    SDL_DisableScreenSaver();
    
    // Set fullscreen if needed
    if(state.fs) {
        if(SDL_SetWindowFullscreen(state.window, 1) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen enabled!");
        }
    }

    // Set rendertargets
    reset_targets();

    // Init hardware renderer
    state.cur_renderer = VIDEO_RENDERER_HW;
    video_hw_init(&state);

    // Get renderer data
    SDL_RendererInfo rinfo;
    SDL_GetRendererInfo(state.renderer, &rinfo);

    // Show some info
    INFO("Video Init OK");
    INFO(" * Driver: %s", SDL_GetCurrentVideoDriver());
    INFO(" * Renderer: %s", rinfo.name);
    INFO(" * Accelerated: %s", (rinfo.flags & SDL_RENDERER_ACCELERATED) ? "Yes" : "No");
    INFO(" * VSync support: %s", (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) ? "Yes" : "No");
    INFO(" * Target support: %s", (rinfo.flags & SDL_RENDERER_TARGETTEXTURE) ? "Yes" : "No");
    return 0;
}

int video_reinit(int window_w, int window_h, int fullscreen, int vsync) {
    // Set window size if necessary
    if(window_w != state.w || window_h != state.h || fullscreen != state.fs) {
        SDL_SetWindowSize(state.window, window_w, window_h);
    }
    
    // Set fullscreen if necessary
    if(fullscreen != state.fs) {
        if(SDL_SetWindowFullscreen(state.window, fullscreen ? SDL_TRUE : SDL_FALSE) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen changed!");
        }
    }

    // VSync change; reset renderer.
    if(vsync != state.vsync) {
        int renderer_flags = SDL_RENDERER_ACCELERATED;
        if(vsync) {
            renderer_flags |= SDL_RENDERER_PRESENTVSYNC;
        }
        SDL_DestroyRenderer(state.renderer);
        state.renderer = SDL_CreateRenderer(state.window, -1, renderer_flags);
        SDL_RenderSetLogicalSize(state.renderer, NATIVE_W, NATIVE_H);
    }

    // Set video state
    state.vsync = vsync;
    state.fs = fullscreen;
    state.w = window_w;
    state.h = window_h;

    // Clear texture cache and reset rendertarget, just in case
    reset_targets();
    tcache_clear();

    state.cb.render_reinit(&state);
    return 0;
}

void video_select_renderer(int renderer) {
    if(renderer == state.cur_renderer) {
        return;
    }
    state.cur_renderer = renderer;
    switch(renderer) {
        case VIDEO_RENDERER_QUIRKS:
            state.cb.render_close(&state);
            video_soft_init(&state);
            break;
        case VIDEO_RENDERER_HW:
            state.cb.render_close(&state);
            video_hw_init(&state);
            break;
    }
}

void video_set_fade(float fade) {
    state.fade = fade;
}

void video_screenshot(image *img) {
    image_create(img, state.w, state.h);
    int ret = SDL_RenderReadPixels(state.renderer, NULL, SDL_PIXELFORMAT_ABGR8888, img->data, img->w * 4);
    if(ret != 0) {
        PERROR("Unable to read pixels from rendertarget: %s", SDL_GetError());
    }
}

void video_force_pal_refresh() {
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
    state.cur_palette->version++;
}

void video_set_base_palette(const palette *src) {
    memcpy(state.base_palette, src, sizeof(palette));
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
    state.cur_palette->version++;
}

palette *video_get_base_palette() {
    return state.base_palette;
}

void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount) {
    memcpy(state.cur_palette->data + dst_start * 3, 
           src->data + src_start * 3, 
           amount * 3);
    state.cur_palette->version++;
}

screen_palette* video_get_pal_ref() {
    return state.cur_palette;
}

void video_render_prepare() {
    // Reset palette
    memcpy(state.cur_palette->data, state.base_palette->data, 768);
    SDL_SetRenderTarget(state.renderer, state.target);
    state.cb.render_prepare(&state);
}

void video_render_background(surface *sur) {
    state.cb.render_background(&state, sur);
}

void video_render_sprite_shadow(surface *sur, int sx, int pal_offset, unsigned int flip_mode) {
    // Position & correct height
    float scale_y = 0.25f;
    SDL_Rect dst;
    dst.w = sur->w;
    dst.h = sur->h * scale_y;
    dst.x = sx;
    dst.y = (190 - sur->h) + (sur->h - dst.h);

    // Flip mode
    SDL_RendererFlip flip = 0;
    if(flip_mode & FLIP_HORIZONTAL) flip |= SDL_FLIP_HORIZONTAL;
    if(flip_mode & FLIP_VERTICAL) flip |= SDL_FLIP_VERTICAL;

    // Render
    state.cb.render_shadow(&state, sur, &dst, pal_offset, flip);
}

void video_render_sprite_tint(
        surface *sur, 
        int sx, 
        int sy, 
        color c, 
        int pal_offset) {

    // Position & correct height
    SDL_Rect dst;
    dst.w = sur->w;
    dst.h = sur->h;
    dst.x = sx;
    dst.y = sy;

    // Render
    state.cb.render_tint(&state, sur, &dst, c, pal_offset);
}

// Wrapper
void video_render_sprite(
        surface *sur, 
        int sx, 
        int sy, 
        unsigned int rendering_mode, 
        int pal_offset) {

    video_render_sprite_flip_scale_opacity(
        sur,
        sx, sy,
        rendering_mode,
        pal_offset,
        FLIP_NONE,
        1.0f,
        255);
}

// Wrapper
void video_render_sprite_flip_scale(
        surface *sur, 
        int sx, 
        int sy, 
        unsigned int rendering_mode, 
        int pal_offset, 
        unsigned int flip_mode, 
        float y_percent) {

    video_render_sprite_flip_scale_opacity(
        sur,
        sx, sy,
        rendering_mode,
        pal_offset,
        flip_mode,
        y_percent,
        255);
}

void video_render_sprite_flip_scale_opacity(
        surface *sur, 
        int sx, 
        int sy, 
        unsigned int rendering_mode, 
        int pal_offset, 
        unsigned int flip_mode, 
        float y_percent, 
        uint8_t opacity) {

    // Position
    SDL_Rect dst;
    dst.w = sur->w;
    dst.h = sur->h * y_percent;
    dst.x = sx;
    dst.y = sy + (sur->h - dst.h) / 2;

    // Flipping
    SDL_RendererFlip flip = 0;
    if(flip_mode & FLIP_HORIZONTAL) flip |= SDL_FLIP_HORIZONTAL;
    if(flip_mode & FLIP_VERTICAL) flip |= SDL_FLIP_VERTICAL;

    // Blend mode
    SDL_BlendMode blend_mode = SDL_BLENDMODE_BLEND;
    if(rendering_mode == BLEND_ADDITIVE)
        blend_mode = SDL_BLENDMODE_ADD;

    // Render
    state.cb.render_fso(&state, sur, &dst, blend_mode, pal_offset, flip, opacity);
}

void video_render_finish() {
    state.cb.render_finish(&state);

    SDL_SetRenderTarget(state.renderer, NULL);
    uint8_t v = 255.0f * state.fade;
    SDL_SetTextureColorMod(state.target, v, v, v);
    SDL_RenderCopy(state.renderer, state.target, NULL, NULL);
    SDL_SetTextureColorMod(state.target, 0xFF, 0xFF, 0xFF);

    SDL_RenderPresent(state.renderer);
    if(!state.vsync) {
        SDL_Delay(1);
    }
}

void video_close() {
    state.cb.render_close(&state);
    SDL_DestroyTexture(state.target);
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    free(state.cur_palette);
    free(state.base_palette);
    INFO("Video deinit.");
}
