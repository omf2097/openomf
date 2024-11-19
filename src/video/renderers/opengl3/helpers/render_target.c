#include <stdlib.h>

#include "utils/allocator.h"
#include "video/renderers/opengl3/helpers/bindings.h"
#include "video/renderers/opengl3/helpers/fbo.h"
#include "video/renderers/opengl3/helpers/render_target.h"
#include "video/renderers/opengl3/helpers/texture.h"

typedef struct render_target {
    GLuint texture_id;
    GLuint fbo_id;
    GLuint tex_unit;
} render_target;

render_target *render_target_create(GLuint tex_unit, int w, int h, GLint internal_format, GLenum format) {
    render_target *target = omf_calloc(1, sizeof(render_target));
    target->texture_id = texture_create(tex_unit, w, h, internal_format, format);
    target->fbo_id = fbo_create(target->texture_id);
    target->tex_unit = tex_unit;
    return target;
}

void render_target_activate(const render_target *target) {
    bindings_bind_fbo(target->fbo_id);
}

void render_target_deactivate(void) {
    bindings_bind_fbo(0);
}

void render_target_free(render_target **target) {
    render_target *obj = *target;
    if(obj != NULL) {
        texture_free(obj->tex_unit, obj->texture_id);
        fbo_free(obj->fbo_id);
        omf_free(obj);
        *target = NULL;
    }
}
