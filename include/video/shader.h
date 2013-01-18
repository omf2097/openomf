#ifndef _SHADER_H
#define _SHADER_H

typedef struct shader_t {
    unsigned int id;
    int type;
} shader;

enum {
    SHADER_VERTEX = 0,
    SHADER_FRAGMENT,
    SHADER_GEOMETRY
};

int shader_create(shader *shader, const char *filename, int type);
void shader_free(shader *shader);
void shader_debug_log(shader *shader);


#endif // _SHADER_H
