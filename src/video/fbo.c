#include "video/fbo.h"
#include "utils/log.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>


const char* fbo_get_status(int code) {
    switch(code) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        return "attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_COUNT_EXT:
        return "layer count";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        return "dimensions";
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        return "formats";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        return "draw_buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        return "read_buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        return "unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_ARB:
        return "layer_targets";
    case GL_FRAMEBUFFER_COMPLETE:
        return "framebuffer complete";
    default:
        return "unknown error";
    }
}
