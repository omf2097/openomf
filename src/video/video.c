#include "video/video.h"
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

SDL_Window *window;
SDL_GLContext glctx;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    // Settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }
    
    // Create context
    glctx = SDL_GL_CreateContext(window);
    if(!glctx) {
        printf("Could not create GL context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }
    
    // Show some info
    printf("[D] Video Init OK\n");
    printf("[D] * SDL2 Driver: %s\n", SDL_GetCurrentVideoDriver());
    printf("[D] * Vendor:      %s\n", glGetString(GL_VENDOR));
    printf("[D] * Renderer:    %s\n", glGetString(GL_RENDERER));
    printf("[D] * Version:     %s\n", glGetString(GL_VERSION));
    
    return 0;
}

void video_render() {
    SDL_GL_SwapWindow(window);
}

void video_close() {
    SDL_GL_DeleteContext(glctx);  
    SDL_DestroyWindow(window);
    printf("[D] Video deinit.\n");
}
