#include "video/shader.h"
#include "utils/log.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>

int shader_create(shader *shader, const char *filename, int type) {
    // Open shader file
    FILE *f = fopen(filename, "rb");
    if(f == NULL) {
        PERROR("Unable to open shader file '%s'.", filename);
        return 1;
    }
    
    // Read contents
    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    rewind(f);
    char *buf = malloc(filesize);
    if(buf == NULL) {
        PERROR("Unable to reserve memory for shader '%s'! %d bytes required.", filename, filesize);
        fclose(f);
        return 1;
    }
    if(fread(buf, 1, filesize, f) != filesize) {
        PERROR("Unable to read shaderfile '%s'.", filename);
        fclose(f);
        free(buf);
        return 1;
    }
    fclose(f);
    
    // Create shader
    switch(type) {
        case SHADER_VERTEX:   shader->id = glCreateShader(GL_VERTEX_SHADER);   break;
        case SHADER_FRAGMENT: shader->id = glCreateShader(GL_FRAGMENT_SHADER); break;
        case SHADER_GEOMETRY: shader->id = glCreateShader(GL_GEOMETRY_SHADER); break;
        default:
            PERROR("Invalid shader type!");
            free(buf);
            return 1;
    }
    glShaderSource(shader->id, 1, (const GLchar**)&buf, 0);
    glCompileShader(shader->id);
    shader->type = type;
    free(buf);
    int compiled = 0;
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compiled);
    return !compiled;
}

void shader_free(shader *shader) {
    glDeleteShader(shader->id);
}

void shader_debug_log(shader *shader) {
    int size = 0;
    glGetShaderiv(shader->id, GL_INFO_LOG_LENGTH, &size);
    if(size > 0) {
        char buf[size];
        glGetShaderInfoLog(shader->id, size, 0, buf);
        DEBUG(buf);
    }
}