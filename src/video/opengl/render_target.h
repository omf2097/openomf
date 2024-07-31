#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

#include "epoxy/gl.h"

typedef struct render_target render_target;

render_target *render_target_create(GLuint tex_unit, int w, int h, GLint internal_format, GLenum format);
void render_target_activate(const render_target *target);
void render_target_deactivate();
void render_target_free(render_target **target);

#endif // RENDER_TARGET_H
