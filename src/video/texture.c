#include "video/texture.h"
#include "utils/log.h"
#include "video/texturelist.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <memory.h>

int texture_upload(texture *tex) {
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->bitmap_w, tex->bitmap_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
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
    tex->w = tex->bitmap_w = w;
    tex->h = tex->bitmap_h = h;
    
    // Reserve texture ID
    glGenTextures(1, &tex->id);
    
    // If data is null, then we create an uninitialized texture
    // Otherwise, copy the raw image data to a buffer.
    tex->data = NULL;
    if(data != NULL) {
        tex->data = malloc(w * h * 4);
        memcpy(tex->data, data, w * h * 4);
    }
    if(!texture_upload(tex)) {
        texturelist_add(tex);
        return 0;
    }
    return 1;
}

int texture_create_from_img(texture *tex, const image *img) {
    return texture_create(tex, img->data, img->w, img->h);
}

void texture_free(texture *tex) {
    texturelist_remove(tex);
    if(tex->data != 0) {
        free(tex->data);
        tex->data = NULL;
    }
    glDeleteTextures(1, &tex->id);
    tex->w = 0;
    tex->h = 0;
}

unsigned int texture_size(texture *tex) {
    if(tex->data == 0) {
        return 0;
    }
    return tex->bitmap_w * tex->bitmap_h * 4;
}

int texture_revalidate(texture *tex) {
    if(!texture_valid(tex)) {
        return texture_upload(tex);
    }
    return 0;
}

int texture_pix_opaque(texture *tex, unsigned int x, unsigned int y) {
    return (tex->data[(x + y * tex->bitmap_w) * 4 + 3] > 0);
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
