#include <assert.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/hashmap.h"
#include "utils/log.h"
#include "utils/sprite_packer.h"
#include "video/renderers/opengl3/helpers/texture.h"
#include "video/renderers/opengl3/helpers/texture_atlas.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} atlas_entry;

static_assert(8 == sizeof(atlas_entry), "atlas_entry should pack into 8 bytes");

typedef struct texture_atlas {
    hashmap items;
    sprite_packer *packer;
    GLuint texture_id;
    uint16_t w;
    uint16_t h;
    GLuint tex_unit;
} texture_atlas;

texture_atlas *atlas_create(GLuint tex_unit, uint16_t width, uint16_t height) {
    texture_atlas *atlas = omf_calloc(1, sizeof(texture_atlas));
    hashmap_create(&atlas->items);
    atlas->packer = sprite_packer_create(width, height);
    atlas->w = width;
    atlas->h = height;
    atlas->tex_unit = tex_unit;
    atlas->texture_id = texture_create(tex_unit, width, height, GL_R8, GL_RED, GL_NEAREST);
    log_debug("Texture atlas %dx%d created", width, height);
    return atlas;
}

void atlas_free(texture_atlas **atlas) {
    texture_atlas *obj = *atlas;
    if(obj != NULL) {
        hashmap_free(&obj->items);
        sprite_packer_free(&obj->packer);
        texture_free(obj->tex_unit, obj->texture_id);
        omf_free(obj);
        *atlas = NULL;
        log_debug("Texture atlas freed");
    }
}

bool atlas_insert(texture_atlas *atlas, const char *bytes, uint16_t w, uint16_t h, uint16_t *nx, uint16_t *ny) {
    sprite_region region;
    if(!sprite_packer_alloc(atlas->packer, w, h, &region)) {
        log_error("Texture atlas has no room for %dx%d area", w, h);
        return false;
    }

    texture_update(atlas->tex_unit, atlas->texture_id, region.x, region.y, w, h, GL_RED, bytes);
    *nx = region.x;
    *ny = region.y;
    return true;
}

bool atlas_get(texture_atlas *atlas, const surface *surface, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h) {
    // First, check if item is already in the texture atlas. If it is, return coords immediately.
    atlas_entry *coords;
    if(hashmap_get_int(&atlas->items, surface->guid, (void **)&coords, NULL) == 0) {
        *x = coords->x;
        *y = coords->y;
        *w = surface->w;
        *h = surface->h;
        return true;
    }

    // If item is NOT in the texture atlas, add it now.
    uint16_t nx, ny;
    if(atlas_insert(atlas, (const char *)surface->data, surface->w, surface->h, &nx, &ny)) {
        *x = nx;
        *y = ny;
        *w = surface->w;
        *h = surface->h;
        atlas_entry cached = {nx, ny, surface->w, surface->h};
        hashmap_put_int(&atlas->items, surface->guid, &cached, sizeof(atlas_entry));
        return true;
    }

    return false;
}

void atlas_reset(texture_atlas *atlas) {
    hashmap_clear(&atlas->items);
    sprite_packer_reset(atlas->packer);
    log_debug("Texture atlas reset");
}
