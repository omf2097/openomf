#ifndef _FBO_H
#define _FBO_H

#include "video/texture.h"
#include "video/rbo.h"

typedef struct fbo_t {
    unsigned int id;
    rbo rbo;
    texture tex;
} fbo;

int fbo_create(fbo *fbo, unsigned int width, unsigned int height);
void fbo_free(fbo *fbo);
void fbo_bind(fbo *fbo);
void fbo_unbind();

#endif // _FBO_H
