#include "video/texture.h"
#include "video/video.h"
#include "video/fbo.h"
#include "video/rbo.h"
#include "video/glextloader.h"
#include "video/shaderprogram.h"
#include "video/shader.h"
#include "utils/log.h"
#include "utils/list.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>

SDL_Window *window;
SDL_GLContext glctx;
fbo target;
unsigned int fullscreen_quad, fullscreen_quad_flipped;
int screen_w, screen_h;

shaderprogram lights;
//shaderprogram xbr;

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
    
    // Load shaders
    shader *lightshow = malloc(sizeof(shader));
    if(shader_create(lightshow, "shaders/light.ps", SHADER_FRAGMENT)) {
        PERROR("Unable to link shader!");
        shader_debug_log(lightshow);
        SDL_DestroyWindow(window);
        return 1;
    }
    shader_debug_log(lightshow);
    
    // Load shaderprogram
    shaderprog_create(&lights);
    shaderprog_attach(&lights, lightshow);
    if(shaderprog_link(&lights)) {
        PERROR("Unable to link shaderprogram!");
        shaderprog_debug_log(&lights);
        SDL_DestroyWindow(window);
        return 1;
    }
    shaderprog_debug_log(&lights);
    
    /*shader *xbrpix = malloc(sizeof(shader));
    if(shader_create(xbrpix, "shaders/xbr.ps", SHADER_FRAGMENT)) {
        PERROR("Unable to link shader!");
        shader_debug_log(xbrpix);
        SDL_DestroyWindow(window);
        return 1;
    }
    shader_debug_log(xbrpix);
    
    shader *xbrver = malloc(sizeof(shader));
    if(shader_create(xbrver, "shaders/xbr.vs", SHADER_VERTEX)) {
        PERROR("Unable to link shader!");
        shader_debug_log(xbrver);
        SDL_DestroyWindow(window);
        return 1;
    }
    shader_debug_log(xbrver);
    
    // Load shaderprogram
    shaderprog_create(&xbr);
    shaderprog_attach(&xbr, xbrpix);
    shaderprog_attach(&xbr, xbrver);
    if(shaderprog_link(&xbr)) {
        PERROR("Unable to link shaderprogram!");
        shaderprog_debug_log(&xbr);
        SDL_DestroyWindow(window);
        return 1;
    }
    shaderprog_debug_log(&xbr);*/
    
    // Enable textures
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    
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
    
    // A nice quad with flipped texture. Screw you OpenGL, and your screwed texture loader!
    fullscreen_quad_flipped = glGenLists(1);
    glNewList(fullscreen_quad_flipped, GL_COMPILE);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, 1.0f, 0.0f); // Top Right
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, 0.0f); // Top Left
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,-1.0f, 0.0f); // Bottom Left
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,-1.0f, 0.0f); // Bottom Right
    glEnd();
    glEndList();
    
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
    DEBUG(" * GLSL:        %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    return 0;
}

void video_set_rendering_mode(int mode) {
    // Set mode
    switch(mode) {
        case BLEND_ADDITIVE:
            // Additive blending, so enable blending and disable alpha testing
            // This shouldn't touch the stencil buffer at all
            glEnable(GL_BLEND);
            glDisable(GL_ALPHA_TEST);
            glEnable(GL_STENCIL_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glStencilFunc(GL_EQUAL, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            break;
        case BLEND_ALPHA_FULL:
            // Full alpha blending. Disable stencil.
            glEnable(GL_BLEND);
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_STENCIL_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        default:
            // Alpha blending. Well, not really blending; we just skip all data where alpha = 0.
            // Set all visible data as 1 on stencil buffer, so that all additive blending effects
            // works on these surfaces.
            glDisable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_STENCIL_TEST);
            glAlphaFunc(GL_GREATER, 0);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glStencilFunc(GL_ALWAYS, 1, 1);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            break;
    }
}

void video_render_prepare() {
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
}

void video_render_background(texture *tex) {
    // Handle background separately
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    texture_bind(tex);
    glCallList(fullscreen_quad_flipped);
    texture_unbind();
}

void video_render_char(texture *tex, int sx, int sy, unsigned char r, unsigned char g, unsigned char b) {
    // Alpha testing
    video_set_rendering_mode(BLEND_ALPHA);

    // Just draw the texture on screen to the right spot.
    float w = tex->w / 160.0f;
    float h = tex->h / 100.0f;
    float x = -1.0 + 2.0f * sx / 320.0f;
    float y = 1.0 - sy / 100.0f - h;
    texture_bind(tex);
    glBegin(GL_QUADS);
        glColor3f(r/255.0f,g/255.0f,b/255.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y+h, 0); // Top Right
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y+h, 0); // Top Left
        glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y,   0); // Bottom Left
        glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y,   0); // Bottom Right
        glColor3f(1.0f,1.0f,1.0f);
    glEnd();
}

void video_render_sprite(texture *tex, int sx, int sy, unsigned int rendering_mode) {
    // Set rendering mode
    video_set_rendering_mode(rendering_mode);
    
    // Just draw the texture on screen to the right spot.
    float w = tex->w / 160.0f;
    float h = tex->h / 100.0f;
    float x = -1.0 + 2.0f * sx / 320.0f;
    float y = 1.0 - sy / 100.0f - h;
    texture_bind(tex);
    glBegin(GL_QUADS);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(x+w, y+h, 0); // Top Right
        glTexCoord2f(0.0f, 0.0f); glVertex3f(x,   y+h, 0); // Top Left
        glTexCoord2f(0.0f, 1.0f); glVertex3f(x,   y,   0); // Bottom Left
        glTexCoord2f(1.0f, 1.0f); glVertex3f(x+w, y,   0); // Bottom Right
    glEnd();
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
    //shaderprog_use(&xbr, 1);
    //shaderprog_use(&lights, 1);
    
    // Draw textured quad
    glCallList(fullscreen_quad);
    
    // TODO: Disable shaders
    //shaderprog_use(&xbr, 0);
    //shaderprog_use(&lights, 0);
    
    // unbind
    texture_unbind();
    
    // Flip screen buffer
    SDL_GL_SwapWindow(window);
}

void video_close() {
    shaderprog_free(&lights);
    fbo_free(&target);
    glDeleteLists(fullscreen_quad, 1);
    SDL_GL_DeleteContext(glctx);  
    SDL_DestroyWindow(window);
    DEBUG("Video deinit.");
}
