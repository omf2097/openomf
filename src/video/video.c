#include "video/video.h"
#include "video/image.h"
#include "utils/log.h"
#include "utils/list.h"
#include "resources/palette.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int w;
    int h;
    int fs;
    int vsync;
    palette *hw_palette;
} video_state;

static video_state state;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    state.w = window_w;
    state.h = window_h;
    state.fs = fullscreen;
    state.vsync = vsync;
    state.hw_palette = malloc(sizeof(palette));

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

    // Get renderer data
    SDL_RendererInfo rinfo;
    SDL_GetRendererInfo(state.renderer, &rinfo);

    // Show some info
    INFO("Video Init OK");
    INFO(" * Driver: %s", SDL_GetCurrentVideoDriver());
    INFO(" * Renderer: %s", rinfo.name);
    INFO(" * Accelerated: %s", (rinfo.flags & SDL_RENDERER_ACCELERATED) ? "Yes" : "No");
    INFO(" * VSync: %s", (rinfo.flags & SDL_RENDERER_PRESENTVSYNC) ? "Yes" : "No");
    return 0;
}

int video_reinit(int window_w, int window_h, int fullscreen, int vsync) {
    // Set fullscreen if necessary
    if(fullscreen != state.fs) {
        if(SDL_SetWindowFullscreen(state.window, fullscreen ? SDL_TRUE : SDL_FALSE) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen changed!");
        }
    }

    // Set window size if necessary
    if(window_w != state.w || window_h != state.h || fullscreen != state.fs) {
        SDL_SetWindowSize(state.window, state.w, state.h);
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
    return 0;
}

void video_screenshot(image *img) {
    image_create(img, state.w, state.h);
    int ret = SDL_RenderReadPixels(state.renderer, NULL, SDL_PIXELFORMAT_RGBA8888, img->data, img->w * 4);
    if(ret != 0) {
        PERROR("Unable to read pixels from rendertarget: %s", SDL_GetError());
    }
}

void video_set_base_palette(palette *src) {
    memcpy(state.hw_palette, src, sizeof(palette));
}

void video_copy_pal_range(palette *src, int start, int end) {
    memcpy(state.hw_palette->data, src->data + start * 3, (end - start) * 3);
}

void video_set_rendering_mode(int mode) {

}

void video_render_prepare() {/*
    // Switch to FBO rendering
    texture_unbind();
    fbo_bind(&target);

    // Set state
    glEnable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);

    // Clear stuff
    glClear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    glClearStencil(0);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0.0f, 0.0f, NATIVE_W, NATIVE_H);
    glLoadIdentity();

    // Set mode
    video_set_rendering_mode(BLEND_ALPHA);
    */
}

void video_render_background(surface *sur) {/*
    // Handle background separately
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    texture_bind(tex);
    glCallList(fullscreen_quad_flipped);
    texture_unbind();
    */
}

void video_render_char(surface *sur, int sx, int sy, color c) {/*
    // Alpha testing
    video_set_rendering_mode(BLEND_ALPHA);

    // Just draw the texture on screen to the right spot.
    float w = tex->w / 160.0f;
    float h = tex->h / 100.0f;
    float x = -1.0 + 2.0f * sx / 320.0f;
    float y = 1.0 - sy / 100.0f - h;
    texture_bind(tex);
    glColor3f(c.r/255.0f, c.g/255.0f, c.b/255.0f);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y+h, 0); // Top Right
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y+h, 0); // Top Left
        glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y,   0); // Bottom Left
        glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y,   0); // Bottom Right
    glEnd();
    glColor3f(1.0f,1.0f,1.0f);*/
}

void video_render_sprite(surface *sur, int sx, int sy, unsigned int rendering_mode) {
    video_render_sprite_flip(sur, sx, sy, rendering_mode, FLIP_NONE);
}

void video_quads(int flip_mode, float x, float y, float w, float h) {/*
    switch(flip_mode) {
        case FLIP_NONE:
            // regular draw
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y+h, 0); // Top Right
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y+h, 0); // Top Left
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y,   0); // Bottom Left
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y,   0); // Bottom Right
            break;
        case FLIP_HORIZONTAL:
            // horizontal flip
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x+w, y+h, 0); // Top Right
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x,   y+h, 0); // Top Left
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x,   y,   0); // Bottom Left
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x+w, y,   0); // Bottom Right
            break;
        case FLIP_VERTICAL:
            // vert flip
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x+w, y+h, 0); // Top Right
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x,   y+h, 0); // Top Left
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x,   y,   0); // Bottom Left
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x+w, y,   0); // Bottom Right
            break;
        case FLIP_VERTICAL|FLIP_HORIZONTAL:
            // both flip
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y+h, 0); // Top Right
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y+h, 0); // Top Left
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y,   0); // Bottom Left
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y,   0); // Bottom Right
            break;
    }*/
}

void video_render_sprite_flip_scale(surface *sur, int sx, int sy, unsigned int rendering_mode, unsigned int flip_mode, float y_percent) {
    /*
    // Set rendering mode
    video_set_rendering_mode(rendering_mode);
    
    // Just draw the texture on screen to the right spot.
    float w = tex->w / 160.0f;
    float h = tex->h / 100.0f;
    float x = -1.0 + 2.0f * sx / 320.0f;
    float y = 1.0 - sy / 100.0f - h;
    float diff =( h - (h * y_percent))/ 2.0f;
    texture_bind(tex);
    glBegin(GL_QUADS);
    video_quads(flip_mode, x, y+diff, w, h * y_percent);
    glEnd();*/
}

void video_render_sprite_flip_alpha(surface *sur, int sx, int sy, unsigned int flip_mode, int alpha) {
    /*
    video_set_rendering_mode(BLEND_ALPHA_CONSTANT);
    float w = tex->w / 160.0f;
    float h = tex->h / 100.0f;
    float x = -1.0 + 2.0f * sx / 320.0f;
    float y = 1.0 - sy / 100.0f - h;
    texture_bind(tex);
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, alpha/255.0f);
    video_quads(flip_mode, x, y, w, h);
    glEnd();
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    */
}

void video_render_colored_quad(int _x, int _y, int _w, int _h, color c) {
    /*
    // Alpha testing
    video_set_rendering_mode(BLEND_ALPHA_FULL);

    // Just draw the quad on screen to the right spot.
    float w = _w / 160.0f;
    float h = _h / 100.0f;
    float x = -1.0 + 2.0f * _x / 320.0f;
    float y = 1.0 - _y / 100.0f - h;
    glDisable(GL_TEXTURE_2D);
    glColor4f(c.r/255.0f, c.g/255.0f, c.b/255.0f, c.a/255.0f);
    glBegin(GL_QUADS);
        glVertex3f(x+w, y+h, 0); // Top Right
        glVertex3f(x,   y+h, 0); // Top Left
        glVertex3f(x,   y,   0); // Bottom Left
        glVertex3f(x+w, y,   0); // Bottom Right
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f,1.0f,1.0f);
    */
}

void video_render_finish() {
    SDL_RenderPresent(state.renderer);
}

void video_close() {
    SDL_DestroyRenderer(state.renderer);
    SDL_DestroyWindow(state.window);
    free(state.hw_palette);
    INFO("Video deinit.");
}
