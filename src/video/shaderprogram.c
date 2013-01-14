#include "video/shaderprogram.h"
#include "utils/log.h"
#include <GL/glew.h>
#include <stdlib.h>

void shaderprog_create(shaderprogram *prog) {
    prog->id = glCreateProgram();
    list_create(&prog->shaders);
    prog->linked = 0;
}

void shaderprog_free(shaderprogram *prog) {
    // Detach shaders
    list_iterator it;
    list_iter(&prog->shaders, &it);
    shader *tmp;
    while((tmp = list_next(&it)) != NULL) {
        glDetachShader(prog->id, tmp->id);
        shader_free(tmp);
        free(tmp);
    }
    list_free(&prog->shaders);
    
    // Destroy program
    glDeleteProgram(prog->id);
}

void shaderprog_attach(shaderprogram *prog, shader *shader) {
    glAttachShader(prog->id, shader->id);
    list_push_last(&prog->shaders, shader);
}

int shaderprog_link(shaderprogram *prog) {
    glLinkProgram(prog->id);
    int status = 0;
    glGetProgramiv(prog->id, GL_LINK_STATUS, &status);
    return !status;
}

void shaderprog_debug_log(shaderprogram *prog) {
    int size = 0;
    glGetProgramiv(prog->id, GL_INFO_LOG_LENGTH, &size);
    if(size > 0) {
        char log[size];
        glGetProgramInfoLog(prog->id, size, 0, log);
        DEBUG(log);
    }
}

void shaderprog_set(shaderprogram *prog, const char *var, int val) {
    unsigned int location = glGetUniformLocation(prog->id, var);
    glUniform1i(location, val);
}

int shaderprog_use(shaderprogram *prog, int use) {
    // Select use / Don't use
    if(use) {
        glUseProgram(prog->id);
    } else {
        glUseProgram(0);
    }
    
    // Check for errors
    if(glGetError() != GL_NO_ERROR) {
        return 1;
    }
    return 0;
}

