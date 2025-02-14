#include "video/renderers/opengl3/sdl_window.h"
#include "game/utils/version.h"
#include "utils/log.h"

#include <epoxy/gl.h>
#include <stdio.h>

bool create_gl_context(SDL_GLContext **context, SDL_Window *window) {
    SDL_GLContext *ctx = SDL_GL_CreateContext(window);
    if(ctx == NULL) {
        log_error("Could not acquire OpenGL context: %s", SDL_GetError());
        return false;
    }
    log_info("OpenGL context acquired!");
    log_info(" * Vendor: %s", glGetString(GL_VENDOR));
    log_info(" * Renderer: %s", glGetString(GL_RENDERER));
    log_info(" * Version: %s", glGetString(GL_VERSION));
    log_info(" * GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
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

bool has_gl_available(int version_major, int version_minor) {
    SDL_Window *w;
    SDL_GLContext *c;
    bool ret = false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, version_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, version_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    if((w = SDL_CreateWindow("", 0, 0, 320, 200, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL)) == NULL) {
        goto exit_0;
    }
    if((c = SDL_GL_CreateContext(w)) == NULL) {
        goto exit_1;
    }

    // All succeeded! We got support!
    ret = true;

    SDL_GL_DeleteContext(c);
exit_1:
    SDL_DestroyWindow(w);
exit_0:
    return ret;
}

bool create_window(SDL_Window **window, int width, int height, bool fullscreen) {
    char title[32];
    snprintf(title, 32, "OpenOMF v%s", get_version_string());

    // Request OpenGL 3.3 core context. This also gives us GLSL 330.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // RGBA8888
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_Window *w = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(w == NULL) {
        log_error("Could not create window: %s", SDL_GetError());
        return false;
    }

    if(fullscreen) {
        if(SDL_SetWindowFullscreen(w, SDL_WINDOW_FULLSCREEN) != 0) {
            log_error("Could not set fullscreen mode: %s", SDL_GetError());
        } else {
            log_info("Fullscreen mode enabled!");
        }
    } else {
        SDL_SetWindowFullscreen(w, 0);
    }

    SDL_DisableScreenSaver();
    *window = w;
    return true;
}

bool resize_window(SDL_Window *window, int width, int height, bool fullscreen) {
    SDL_SetWindowSize(window, width, height);
    if(SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0) < 0) {
        log_error("Could not set fullscreen mode: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool set_vsync(bool enable) {
    // If we don't want to enable vsync, just log and okay out.
    if(!enable) {
        log_info("VSYNC is disabled!");
        return true;
    }
    // Try for adaptive vsync first.
    if(SDL_GL_SetSwapInterval(-1) == 0) {
        log_info("Adaptive VSYNC enabled!");
        return true;
    } else {
        log_error("Adaptive VSYNC not supported: %s", SDL_GetError());
    }
    // Fallback to normal, static vsync
    if(SDL_GL_SetSwapInterval(1) == 0) {
        log_info("Non-adaptive VSYNC enabled!");
        return true;
    } else {
        log_error("VSYNC not supported: %s", SDL_GetError());
    }
    // Fallback to no vsync, in which case we do SDL_Delay.
    if(SDL_GL_SetSwapInterval(0) == 0) {
        log_info("VSYNC is disabled!");
        return true;
    } else {
        log_error("Unable to set any VSYNC mode -- something is really broken.");
    }
    // We're out of fallbacks to give -- fail.
    return false;
}
