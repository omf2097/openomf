#include <string.h>

#include "utils/hashmap.h"
#include "video/texture_atlas.h"

// WIP

typedef struct {
    uint64_t id;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} atlas_item;

typedef struct texture_atlas_t {
    hashmap items;
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

void atlas_create(texture_atlas *atlas, uint16_t width, uint16_t height) {
    memset(atlas, 0, sizeof(texture_atlas));
    hashmap_create(&atlas->items, 10);
    atlas->w = width;
    atlas->h = height;
    atlas->texture_id = create_atlas_texture(width, height);
}

void atlas_free(texture_atlas *atlas) {
    hashmap_free(&atlas->items);
    glDeleteTextures(1, &atlas->texture_id);
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