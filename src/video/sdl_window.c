#include "video/sdl_window.h"
#include "utils/log.h"

#include <epoxy/gl.h>

bool create_gl_context(SDL_GLContext **context, SDL_Window *window) {
    SDL_GLContext *ctx = SDL_GL_CreateContext(window);
    if(ctx == NULL) {
        PERROR("Could not acquire OpenGL context: %s", SDL_GetError());
        return false;
    }
    INFO("OpenGL context acquired!");
    INFO(" * Vendor: %s", glGetString(GL_VENDOR));
    INFO(" * Renderer: %s", glGetString(GL_RENDERER));
    INFO(" * Version: %s", glGetString(GL_VERSION));
    INFO(" * GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    *context = ctx;
    return true;
}

/** Fake out gluOrtho2d
 * http://en.wikipedia.org/wiki/Orthographic_projection_(geometry)
 */
void ortho2d(GLfloat *matrix, float left, float right, float bottom, float top) {
    const GLfloat z_near = -1.0f;
    const GLfloat z_far = 1.0f;
    const GLfloat inv_z = 1.0f / (z_far - z_near);
    const GLfloat inv_y = 1.0f / (top - bottom);
    const GLfloat inv_x = 1.0f / (right - left);
    *matrix++ = 2.0f * inv_x;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = 2.0f * inv_y;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = 0.0f;
    *matrix++ = -2.0f * inv_z;
    *matrix++ = 0.0f;
    *matrix++ = -(right + left) * inv_x;
    *matrix++ = -(top + bottom) * inv_y;
    *matrix++ = -(z_far + z_near) * inv_z;
    *matrix++ = 1.0f;
}

bool create_window(SDL_Window **window, int width, int height, bool fullscreen) {
    char title[32];
    snprintf(title, 32, "OpenOMF v%d.%d.%d", V_MAJOR, V_MINOR, V_PATCH);

    // Request OpenGL 3.3 core context. This also gives us GLSL 330.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // TODO: Probably not required
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // RGBA8888
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Window *w = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(w == NULL) {
        PERROR("Could not create window: %s", SDL_GetError());
        return false;
    }

    if(fullscreen) {
        if(SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN) != 0) {
            PERROR("Could not set fullscreen mode: %s", SDL_GetError());
        } else {
            INFO("Fullscreen mode enabled!");
        }
    } else {
        SDL_SetWindowFullscreen(w, 0);
    }

    SDL_DisableScreenSaver();
    *window = w;
    return true;
}

bool enable_vsync() {
    // Try for adaptive vsync first.
    if(SDL_GL_SetSwapInterval(-1) == 0) {
        INFO("Adaptive VSYNC enabled!");
        return true;
    } else {
        PERROR("Adaptive VSYNC not supported: %s", SDL_GetError());
    }
    // Fallback to normal, static vsync
    if(SDL_GL_SetSwapInterval(1) == 0) {
        INFO("Non-adaptive VSYNC enabled!");
        return true;
    } else {
        PERROR("VSYNC not supported: %s", SDL_GetError());
    }
    // Fallback to no vsync, in which case we do SDL_Delay.
    if(SDL_GL_SetSwapInterval(0) == 0) {
        INFO("VSYNC is disabled! Falling back to delay sleep.");
        return true;
    } else {
        PERROR("Unable to set any VSYNC mode -- something is really broken.");
    }
    // We're out of fallbacks to give -- fail.
    return false;
}
