#ifndef _SHADERPROGRAM_H
#define _SHADERPROGRAM_H

#include "utils/list.h"
#include "video/shader.h"

typedef struct shaderprogram_t {
    unsigned int id;
    list shaders;
    int linked;
} shaderprogram;

void shaderprog_create(shaderprogram *prog);
void shaderprog_free(shaderprogram *prog);
void shaderprog_attach(shaderprogram *prog, shader *shader);
int  shaderprog_link(shaderprogram *prog);
void shaderprog_debug_log(shaderprogram *prog);
void shaderprog_set(shaderprogram *prog, const char *var, int val);
int  shaderprog_use(shaderprogram *prog, int use);

#endif // _SHADERPROGRAM_H