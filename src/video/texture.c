#include "video/texture.h"
#include "utils/log.h"
#include "video/texturelist.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <memory.h>

int texture_upload(texture *tex) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->w, tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
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
    if(tex->w * tex->h * 4 >= 1221988320) {
        DEBUG("ERROR: W: %d H: %d", tex->w, tex->h);
    }
    
    // Reserve texture ID
    glGenTextures(1, &tex->id);
    
    // If data is null, then we create an uninitialized texture
    // Otherwise, copy the raw image data to a buffer.
    if(data == 0) {
        tex->data = 0;
    } else {
        tex->data = malloc(w * h * 4);
        memcpy(tex->data, data, w * h * 4);
    }
    if(!texture_upload(tex)) {
        texturelist_add(tex);
        return 0;
    }
    return 1;
}

void texture_free(texture *tex) {
    texturelist_remove(tex);
    if(tex->data != 0) {
        free(tex->data);
        tex->data = 0;
    }
    glDeleteTextures(1, &tex->id);
    tex->w = 0;
    tex->h = 0;
}

unsigned int texture_size(texture *tex) {
    if(tex->data == 0) {
        return 0;
    }
    return tex->w * tex->h * 4;
}

int texture_revalidate(texture *tex) {
    if(!texture_valid(tex)) {
        return texture_upload(tex);
    }
    return 0;
}

int texture_pix_opaque(texture *tex, unsigned int x, unsigned int y) {
    return (tex->data[(x + y * tex->w) * 4 + 3] > 0);
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
