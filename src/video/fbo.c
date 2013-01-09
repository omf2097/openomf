#include "video/fbo.h"
#include "utils/log.h"
#include <GL/glew.h>


const char* fbo_get_status(int code) {
    switch(code) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        return "draw_buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        return "read_buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        return "layer_targets";
    case GL_FRAMEBUFFER_COMPLETE:
        return "framebuffer complete";
    default:
        return "unknown error";
    }
}
