#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/hashmap.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/opengl/texture_atlas.h"

// WIP

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} __attribute__((packed)) zone;

typedef struct texture_atlas {
    hashmap items;
    vector free_space;
    GLuint texture_id;
    uint16_t w;
    uint16_t h;
} texture_atlas;

static GLuint create_atlas_texture(uint16_t width, uint16_t height) {
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
    return texture_id;
}

static void update_texture(GLuint texture_id, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const char *bytes) {
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RG8, GL_UNSIGNED_BYTE, bytes);
}

texture_atlas *atlas_create(uint16_t width, uint16_t height) {
    texture_atlas *atlas = omf_calloc(1, sizeof(texture_atlas));
    hashmap_create(&atlas->items);
    vector_create(&atlas->free_space, sizeof(zone));
    atlas->w = width;
    atlas->h = height;
    atlas->texture_id = create_atlas_texture(width, height);
    DEBUG("Texture atlas %dx%d created", width, height);
    return atlas;
}

void atlas_free(texture_atlas **atlas) {
    texture_atlas *obj = *atlas;
    hashmap_free(&obj->items);
    vector_free(&obj->free_space);
    glDeleteTextures(1, &obj->texture_id);
    omf_free(obj);
    *atlas = NULL;
    DEBUG("Texture atlas freed");
}

void atlas_insert(texture_atlas *atlas, uint64_t id, const char *bytes, uint16_t width, uint16_t height) {
    // TODO: Implement
    update_texture(atlas->texture_id, 0, 0, width, height, bytes);
}

void atlas_clean(texture_atlas *atlas) {
    // TODO: Implement
}

GLuint atlas_get_texture_id(texture_atlas *atlas) {
    return atlas->texture_id;
}