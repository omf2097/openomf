#include "minlib/minlib_internal.h"

#ifndef MIN_LIB_BUILD
#error "gl_loader.c is part of MIN_LIB_BUILD"
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <epoxy/gl.h>

static PROC gl_get_proc(const char *name) {
    PROC proc = wglGetProcAddress(name);
    if(proc == NULL || proc == (PROC)1 || proc == (PROC)2 || proc == (PROC)3 || proc == (PROC)-1) {
        static HMODULE opengl = NULL;
        if(opengl == NULL) {
            opengl = LoadLibraryA("opengl32.dll");
        }
        if(opengl != NULL) {
            proc = GetProcAddress(opengl, name);
        }
    }
    return proc;
}

#define GL_FUNC(ret, name, args) PFN_##name name = NULL;
#include <epoxy/gl_funcs.inc>
#undef GL_FUNC

bool gl_minlib_load(void) {
#define GL_FUNC(ret, name, args)                                                                                       \
    name = (PFN_##name)gl_get_proc(#name);                                                                             \
    if(name == NULL) {                                                                                                 \
        sdl_set_error("OpenGL entry point %s is missing", #name);                                                      \
        return false;                                                                                                  \
    }
#include <epoxy/gl_funcs.inc>
#undef GL_FUNC
    return true;
}
