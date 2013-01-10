#include "video/video.h"
#include "video/fbo.h"
#include "video/texture.h"
#include "video/rbo.h"
#include "video/glextloader.h"
#include "utils/log.h"
#include "utils/list.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

SDL_Window *window;
SDL_GLContext glctx;
fbo target;
unsigned int fullscreen_quad;
list render_list;
texture *background_texture;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    // Settings
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);                                               
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);   

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
    
    // A nice quad. Screw you, OpenGL 3!
    fullscreen_quad = glGenLists(1);
    glNewList(fullscreen_quad, GL_COMPILE);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, 1.0f, 0.0f); // Top Right
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 0.0f); // Top Left
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,-1.0f, 0.0f); // Bottom Left
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,-1.0f, 0.0f); // Bottom Right
    glEnd();
    glEndList();
    
    // Render target FBO
    if(fbo_create(&target, NATIVE_W, NATIVE_H)) {
        SDL_DestroyWindow(window);
        return 1;
    }
    
    // BG Tex
    background_texture = 0;
    
    // Show some info
    DEBUG("Video Init OK");
    DEBUG(" * SDL2 Driver: %s", SDL_GetCurrentVideoDriver());
    DEBUG(" * Vendor:      %s", glGetString(GL_VENDOR));
    DEBUG(" * Renderer:    %s", glGetString(GL_RENDERER));
    DEBUG(" * Version:     %s", glGetString(GL_VERSION));
    DEBUG(" * GLSL:        %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return 0;
}

void video_render_prepare() {
    // Initialize the list of stuff to be rendered
    list_create(&render_list);

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

void video_set_background(texture *tex) {
    background_texture = tex;
}

void video_render_sprite(gl_sprite *sprite) {
    list_push_last(&render_list, sprite);
}

void video_render_finish() {
    // Handle background separately
    if(background_texture) {
        glStencilFunc(GL_ALWAYS, 0, 0);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        texture_bind(background_texture);
        glCallList(fullscreen_quad);
    }

    // Render sprites etc. here from list
    list_iterator it;
    list_iter(&render_list, &it);
    gl_sprite *sprite;
    while((sprite = list_next(&it)) != 0) {
        switch(sprite->rendering_mode) {
        case BLEND_ADDITIVE:
            // Additive blending, so enable blending and disable alpha testing
            // This shouldn't touch the stencil buffer at all
            glEnable(GL_BLEND);
            glDisable(GL_ALPHA_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glStencilFunc(GL_EQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;
        case BLEND_ALPHA:
            // Alpha blending. Well, not really blending; we just skip all data where alpha = 0.
            // Set all visible data as 1 on stencil buffer, so that all additive blending effects
            // works on these surfaces.
            glDisable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glAlphaFunc(GL_GREATER, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
        }

        // Just draw the texture on screen to the right spot.
        int x = sprite->x;
        int y = sprite->y;
        int w = sprite->tex.w;
        int h = sprite->tex.h;
        texture_bind(&sprite->tex);
        glBegin(GL_QUADS);
            glColor3f(1.0f,0.0f,0.0f); 
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y+h, 0.0f); // Top Right
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y+h, 0.0f); // Top Left
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y,   0.0f); // Bottom Left
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y,   0.0f); // Bottom Right
        glEnd();
    }    
    list_free(&render_list);

    // Render to screen instead of FBO
    fbo_unbind();

    // Clear stuff
    glDisable(GL_STENCIL_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, 640, 400);
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
    
    // Draw textured quad
    glCallList(fullscreen_quad);
    
    // TODO: Disable shaders
    
    // unbind
    texture_unbind();
    
    // Flip screen buffer
    SDL_GL_SwapWindow(window);
}

void video_close() {
    fbo_free(&target);
    glDeleteLists(fullscreen_quad, 1);
    SDL_GL_DeleteContext(glctx);  
    SDL_DestroyWindow(window);
    DEBUG("Video deinit.");
}
