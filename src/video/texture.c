#include "video/texture.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#define GL3_PROTOTYPES 1
#include <GL/gl.h>

void texture_internal_create(unsigned int *id, int w, int h, const char *data) {
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#ifdef DEBUGMODE
    if(glGetError() != GL_NO_ERROR) {
        PERROR("Error while creating texture!");
    }
#endif
}

void texture_create(texture *tex, sd_rgba_image *img) {
    tex->original = img;
    texture_internal_create(&tex->id, img->w, img->h, img->data);
}

void texture_free(texture *tex) {
    glDeleteTextures(1, &tex->id);
    sd_rgba_image_delete(tex->original);
}

void texture_validate(texture *tex) {
    if(glIsTexture(tex->id) != GL_TRUE) {
        texture_internal_create(&tex->id, tex->original->w, tex->original->h, tex->original->data);
    }
}
