#include "video/texture.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include <GL/glew.h>

void texture_create(texture *tex, const char *data, unsigned int w, unsigned int h) {
    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef DEBUGMODE
    if(glGetError() != GL_NO_ERROR) {
        PERROR("Error while creating texture!");
    }
#endif
}

void texture_free(texture *tex) {
    glDeleteTextures(1, &tex->id);
}

int texture_valid(texture *tex) {
    return glIsTexture(tex->id);
}

void texture_bind(texture *tex) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
}

void texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}