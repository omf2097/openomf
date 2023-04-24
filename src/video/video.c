#include <SDL.h>
#include <epoxy/gl.h>
#include <stdio.h>
#include <stdlib.h>

#include "formats/palette.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/image.h"
#include "video/tcache.h"
#include "video/video.h"
#include "video/video_state.h"

static video_state g_video_state;

bool create_window(int width, int height, bool fullscreen) {
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

    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(window == NULL) {
        PERROR("Could not create window: %s", SDL_GetError());
        return false;
    }

    if(fullscreen) {
        if(SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) != 0) {
            PERROR("Could not set fullscreen mode!");
        } else {
            INFO("Fullscreen mode enabled!");
        }
    } else {
        SDL_SetWindowFullscreen(window, 0);
    }

    SDL_DisableScreenSaver();
    g_video_state.window = window;
    return true;
}

bool create_gl_context(void) {
    SDL_GLContext *context = SDL_GL_CreateContext(g_video_state.window);
    if(context == NULL) {
        PERROR("Could not acquire OpenGL context: %s", SDL_GetError());
        return false;
    }
    INFO("OpenGL context acquired!");
    INFO(" * Vendor: %s", glGetString(GL_VENDOR));
    INFO(" * Renderer: %s", glGetString(GL_RENDERER));
    INFO(" * Version: %s", glGetString(GL_VERSION));
    INFO(" * GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    g_video_state.gl_context = context;
    return true;
}

void print_log(const char *buffer, long len, const char *header) {
    str log;
    str sub;

    // Make sure the string always ends in one \n
    str_from_buf(&log, buffer, len);
    str_rstrip(&log);
    str_append_c(&log, "\n");

    // Print line by line
    size_t pos = 0, last = 0;
    PERROR("--- %s ---", header);
    while(str_find_next(&log, '\n', &pos)) {
        str_from_slice(&sub, &log, last, pos - 1);
        PERROR("%s", str_c(&sub));
        str_free(&sub);
        last = ++pos;
    }
    PERROR("--- end log ---", header);

    str_free(&log);
}

void print_shader_log(const GLuint shader, const char *header) {
    if(!glIsShader(shader)) {
        PERROR("Attempted to print logs for shader %d: not a shader", shader);
        return;
    }

    int buffer_size = 0, read_size = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &buffer_size);
    char *buffer_data = omf_calloc(buffer_size, 1);
    glGetShaderInfoLog(shader, buffer_size, &read_size, buffer_data);
    print_log(buffer_data, read_size, header);
    omf_free(buffer_data);
}

void print_program_log(const GLuint program) {
    if(!glIsProgram(program)) {
        PERROR("Attempted to print logs for program %d: not a program", program);
        return;
    }

    int buffer_size = 0, read_size = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buffer_size);
    char *buffer_data = omf_calloc(buffer_size, 1);
    glGetProgramInfoLog(program, buffer_size, &read_size, buffer_data);
    print_log(buffer_data, read_size, "program");
    omf_free(buffer_data);
}

bool load_shader(GLuint program_id, GLenum shader_type, const char *shader_file) {
    str shader_path;
    str shader_source;

    str_from_format(&shader_path, "%s%s", pm_get_local_path(SHADER_PATH), shader_file);
    str_from_file(&shader_source, str_c(&shader_path));
    const char *c_str = str_c(&shader_source);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &c_str, NULL);
    glCompileShader(shader);

    str_free(&shader_source);
    str_free(&shader_path);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        PERROR("Compilation error for shader %d (file=%s)", shader, shader_file);
        print_shader_log(shader, shader_file);
        return false;
    }
    DEBUG("Compilation succeeded for shader %d (file=%s)", shader, shader_file);
    glAttachShader(program_id, shader);
    return true;
}

void delete_program(GLuint program_id) {
    GLsizei attached_count = 0;
    glGetProgramiv(program_id, GL_ATTACHED_SHADERS, &attached_count);
    GLuint shaders[attached_count];
    glGetAttachedShaders(program_id, attached_count, NULL, shaders);
    for(int i = 0; i < attached_count; i++) {
        DEBUG("Shader %d deleted", i);
        glDeleteShader(shaders[i]); // Mark for removal, glDeleteProgram will handle deletion.
    }
    glDeleteProgram(program_id);
    DEBUG("Program %d deleted", program_id);
}

bool create_program(GLuint *program_id, const char *vertex_shader, const char *fragment_shader) {
    GLuint id = glCreateProgram();
    if(!load_shader(id, GL_VERTEX_SHADER, vertex_shader))
        goto error_0;
    if(!load_shader(id, GL_FRAGMENT_SHADER, fragment_shader))
        goto error_0;

    glLinkProgram(id);
    GLint status = GL_TRUE;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        PERROR("Compilation error for program %d (vert=%s, frag=%s)", id, vertex_shader, fragment_shader);
        print_program_log(id);
        goto error_0;
    }

    *program_id = id;
    DEBUG("Compilation succeeded for program %d (vert=%s, frag=%s)", id, vertex_shader, fragment_shader);
    return true;

error_0:
    delete_program(id);
    return false;
}

bool enable_vsync(void) {
    // Try for adaptive vsync first.
    if(SDL_GL_SetSwapInterval(-1) == 0) {
        INFO("Adaptive VSYNC enabled!");
        return true;
    } else {
        PERROR("Adaptive VSYNC not supported: ", SDL_GetError());
    }
    // Fallback to normal, static vsync
    if(SDL_GL_SetSwapInterval(1) == 0) {
        INFO("Non-adaptive VSYNC enabled!");
        return true;
    } else {
        PERROR("VSYNC not supported: ", SDL_GetError());
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

int video_init(int window_w, int window_h, bool fullscreen, bool vsync) {
    g_video_state.screen_w = window_w;
    g_video_state.screen_h = window_h;
    g_video_state.fullscreen = fullscreen;
    g_video_state.vsync = vsync;
    g_video_state.fade = 1.0f;
    g_video_state.target_move_x = 0;
    g_video_state.target_move_y = 0;
    g_video_state.render_bg_separately = true;

    // Clear palettes
    g_video_state.base_palette = omf_calloc(1, sizeof(palette));
    g_video_state.extra_palette = omf_calloc(1, sizeof(screen_palette));
    g_video_state.screen_palette = omf_calloc(1, sizeof(screen_palette));
    g_video_state.extra_palette->version = 0;
    g_video_state.screen_palette->version = 1;

    if(!create_window(window_w, window_h, fullscreen)) {
        goto error_0;
    }
    if(!create_gl_context()) {
        goto error_1;
    }
    if(!enable_vsync()) {
        goto error_2;
    }
    if(!create_program(&g_video_state.shader_prog, "direct.vert", "direct.frag")) {
        goto error_2;
    }

    INFO("OpenGL Renderer initialized!");
    return 0;

error_2:
    SDL_GL_DeleteContext(g_video_state.gl_context);

error_1:
    SDL_DestroyWindow(g_video_state.window);

error_0:
    omf_free(g_video_state.screen_palette);
    omf_free(g_video_state.extra_palette);
    omf_free(g_video_state.base_palette);
    return 1;
}

void video_reinit_renderer(void) {

}

int video_reinit(int window_w, int window_h, bool fullscreen, bool vsync) {
    return 0;
}

// Called on every game tick
void video_tick() {
}

// Called after frame has been rendered
void video_render_finish(void) {

    // Flip buffers. If vsync is off, we should sleep here
    // so hat our main loop doesn't eat up all cpu :)
    SDL_GL_SwapWindow(g_video_state.window);
    if(!g_video_state.vsync) {
        SDL_Delay(1);
    }
}

void video_close(void) {
    delete_program(g_video_state.shader_prog);
    SDL_GL_DeleteContext(g_video_state.gl_context);
    SDL_DestroyWindow(g_video_state.window);
    omf_free(g_video_state.screen_palette);
    omf_free(g_video_state.extra_palette);
    omf_free(g_video_state.base_palette);
    INFO("Video renderer closed.");
}

void video_move_target(int x, int y) {
    g_video_state.target_move_x = x;
    g_video_state.target_move_y = y;
}

void video_get_state(int *w, int *h, int *fs, int *vsync) {
    if(w != NULL) {
        *w = g_video_state.screen_w;
    }
    if(h != NULL) {
        *h = g_video_state.screen_h;
    }
    if(fs != NULL) {
        *fs = g_video_state.fullscreen;
    }
    if(vsync != NULL) {
        *vsync = g_video_state.vsync;
    }
}

void video_set_fade(float fade) {
    g_video_state.fade = fade;
}

int video_screenshot(image *img) {
    return 1;
}

int video_area_capture(surface *sur, int x, int y, int w, int h) {
    return 1;
}

void video_force_pal_refresh(void) {
    memcpy(g_video_state.screen_palette->data, g_video_state.base_palette->data, 768);
    memcpy(g_video_state.extra_palette->data, g_video_state.base_palette->data, 768);
    g_video_state.extra_palette->version++;
    g_video_state.screen_palette->version++;
}

void video_set_base_palette(const palette *src) {
    memcpy(g_video_state.base_palette, src, sizeof(palette));
    video_force_pal_refresh();
}

palette *video_get_base_palette(void) {
    return g_video_state.base_palette;
}

void video_copy_pal_range(const palette *src, int src_start, int dst_start, int amount) {
    memcpy(g_video_state.screen_palette->data + dst_start * 3, src->data + src_start * 3, amount * 3);
    g_video_state.screen_palette->version++;
}

void video_copy_base_pal_range(const palette *src, int src_start, int dst_start, int amount) {
    memcpy(state.base_palette->data + dst_start, src->data + src_start, amount * 3);
    video_force_pal_refresh();
}

screen_palette *video_get_pal_ref(void) {
    return g_video_state.screen_palette;
}

void video_render_prepare(void) {
    // Reset palette
    memcpy(g_video_state.screen_palette->data, g_video_state.base_palette->data, 768);
}

void video_render_bg_separately(bool separate) {
    g_video_state.render_bg_separately = separate;
}

void video_render_background(surface *sur) {
}

static void render_sprite_fsot(video_state *state, surface *sur, SDL_Rect *dst, SDL_BlendMode blend_mode,
                               int pal_offset, SDL_RendererFlip flip_mode, uint8_t opacity, color color_mod) {
    return;
}

void video_render_sprite_tint(surface *sur, int sx, int sy, color c, int pal_offset) {

    video_render_sprite_flip_scale_opacity_tint(sur, sx, sy, BLEND_ALPHA, pal_offset, FLIP_NONE, 1.0f, 1.0f, 255, c);
}

// Wrapper
void video_render_sprite(surface *sur, int sx, int sy, unsigned int rendering_mode, int pal_offset) {

    video_render_sprite_flip_scale_opacity(sur, sx, sy, rendering_mode, pal_offset, FLIP_NONE, 1.0f, 1.0f, 255);
}

void video_render_sprite_flip_scale_opacity(surface *sur, int sx, int sy, unsigned int rendering_mode, int pal_offset,
                                            unsigned int flip_mode, float x_percent, float y_percent, uint8_t opacity) {

    video_render_sprite_flip_scale_opacity_tint(sur, sx, sy, rendering_mode, pal_offset, flip_mode, x_percent,
                                                y_percent, opacity, color_create(0xFF, 0xFF, 0xFF, 0xFF));
}

void video_render_sprite_size(surface *sur, int sx, int sy, int sw, int sh) {
    SDL_Rect dst;
    dst.w = sw;
    dst.h = sh;
    dst.x = sx;
    dst.y = sy;

    // Render
    render_sprite_fsot(&g_video_state, sur, &dst, SDL_BLENDMODE_BLEND, 0, 0, 0xFF,
                       color_create(0xFF, 0xFF, 0xFF, 0xFF)); // tint
}

void video_render_sprite_flip_scale_opacity_tint(surface *sur, int sx, int sy, unsigned int rendering_mode,
                                                 int pal_offset, unsigned int flip_mode, float x_percent,
                                                 float y_percent, uint8_t opacity, color tint) {

    // Position
    SDL_Rect dst;
    dst.w = sur->w * x_percent;
    dst.h = sur->h * y_percent;
    dst.x = sx;
    dst.y = sy + (sur->h - dst.h) / 2;

    // Flipping
    SDL_RendererFlip flip = 0;
    if(flip_mode & FLIP_HORIZONTAL)
        flip |= SDL_FLIP_HORIZONTAL;
    if(flip_mode & FLIP_VERTICAL)
        flip |= SDL_FLIP_VERTICAL;

    // Select SDL blend mode
    SDL_BlendMode blend_mode = (rendering_mode == BLEND_ALPHA) ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD;

    render_sprite_fsot(&g_video_state, sur, &dst, blend_mode, pal_offset, flip, opacity, tint);
}
