#include "video/video.h"
#include "video/fbo.h"
#include "video/texture.h"
#include "video/rbo.h"
#include "video/glextloader.h"
#include "utils/log.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

SDL_Window *window;
SDL_GLContext glctx;
fbo target;

unsigned int screen_w;
unsigned int screen_h;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    screen_w = window_w;
    screen_h = window_h;

    // Settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);                                               
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);   

    // Open window
    window = SDL_CreateWindow(
        "OpenOMF",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_w,
        window_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if(!window) {
        PERROR("Could not create window: %s", SDL_GetError());
        return 1;
    }
    
    // Set fullscreen if needed
    if(fullscreen) {
        if(SDL_SetWindowFullscreen(window, 1) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            DEBUG("Fullscreen enabled!");
        }
    }
    
    // Create context
    glctx = SDL_GL_CreateContext(window);
    if(!glctx) {
        PERROR("Could not create GL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }
    
    // Initialize GL extension loader
    if(glext_init()) {
        SDL_DestroyWindow(window);
        return 1;
    }
    
    // Set VSync
    if(vsync) {
        if(SDL_GL_SetSwapInterval(1) != 0) {
            PERROR("Could not enable VSync!");
        } else {
            DEBUG("VSync enabled!");
        }
    }
    
    // Disable depth testing etc (not needed), enable textures
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    
    // Render target FBO
    if(fbo_create(&target, NATIVE_W, NATIVE_H)) {
        SDL_DestroyWindow(window);
        return 1;
    }
    
    // Show some info
    DEBUG("Video Init OK");
    DEBUG(" * SDL2 Driver: %s", SDL_GetCurrentVideoDriver());
    DEBUG(" * Vendor:      %s", glGetString(GL_VENDOR));
    DEBUG(" * Renderer:    %s", glGetString(GL_RENDERER));
    DEBUG(" * Version:     %s", glGetString(GL_VERSION));
    return 0;
}

void video_render_prepare() {
    // Switch to FBO rendering
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
    glViewport(0, 0, NATIVE_W, NATIVE_H);
    glLoadIdentity();
}

void video_render_artistry() {

}

void video_render_finish() {
    // Render to screen instead of FBO
    fbo_unbind();

    // Clear stuff
    glDisable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, screen_w, screen_h);
    glLoadIdentity();

    // Disable blending & alpha testing
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);

    // Pick texture, select state
    texture_bind(&target.tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // TODO: Enable shaders
    // TODO: Render fullscreen quad or something
    // TODO: Disable shaders
    
    // unbind
    texture_unbind();
    
    // Flip screen buffer
    SDL_GL_SwapWindow(window);
}

void video_render() {
    video_render_prepare();
    video_render_artistry();
    video_render_finish();
}

void video_close() {
    fbo_free(&target);
    SDL_GL_DeleteContext(glctx);  
    SDL_DestroyWindow(window);
    DEBUG("Video deinit.");
}
