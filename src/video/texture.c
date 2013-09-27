#include "video/texture.h"
#include "utils/log.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <memory.h>

int texture_upload(texture *tex, const char* data) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef DEBUGMODE
    if(glGetError() != GL_NO_ERROR) {
        PERROR("Error while creating texture!");
        return 1;
    }
#endif
    return 0;
}

int texture_create(texture *tex, const char *data, unsigned int w, unsigned int h) {
    tex->w = w;
    tex->h = h;
    tex->id = 0;
    glGenTextures(1, &tex->id);
    return texture_upload(tex, data);
}

int texture_create_from_img(texture *tex, const image *img) {
    return texture_create(tex, img->data, img->w, img->h);
}

void texture_free(texture *tex) {
    if(tex->id != 0) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
        tex->w = 0;
        tex->h = 0;
    }
}

unsigned int texture_size(texture *tex) {
    return tex->w * tex->h * 4;
}

int texture_is_valid(texture *tex) {
    return glIsTexture(tex->id);
}

void texture_bind(texture *tex) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
}

void texture_unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}
