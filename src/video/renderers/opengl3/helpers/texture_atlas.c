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
    GLenum internal_fmt = (sizeof(vga_pixel) == 2) ? GL_R16UI : GL_R8UI;
    GLenum type = (sizeof(vga_pixel) == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
    atlas->texture_id = texture_create(tex_unit, width, height, internal_fmt, GL_RED_INTEGER, type, GL_NEAREST);
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

bool atlas_insert(texture_atlas *atlas, const vga_pixel *data, uint16_t w, uint16_t h, uint16_t *nx, uint16_t *ny) {
    sprite_region region;
    if(!sprite_packer_alloc(atlas->packer, w, h, &region)) {
        log_error("Texture atlas has no room for %dx%d area", w, h);
        return false;
    }

    GLenum type = (sizeof(vga_pixel) == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
    texture_update(atlas->tex_unit, atlas->texture_id, region.x, region.y, w, h, GL_RED_INTEGER, type, data);
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
#ifdef USE_EXTENDED_PALETTE
    // When the surface has a remap table, apply it to redirect palette indices
    // into extended zones (256-1023) for mod sprites.
    const vga_remap_table *remap = surface->remap;
    const vga_pixel *data;
    vga_pixel *buf = NULL;
    if(remap != NULL) {
        int pixels = surface->w * surface->h;
        buf = omf_malloc(pixels * sizeof(vga_pixel));
        for(int i = 0; i < pixels; i++) {
            buf[i] = remap->data[surface->data[i]];
        }
        data = buf;
        fprintf(stderr, "ATLAS REMAP: surface guid=%u, %d pixels, first pixel: %d -> %d\n",
                surface->guid, pixels, surface->data[0], buf[0]);
    } else {
        data = surface->data;
    }
#else
    const vga_pixel *data = surface->data;
#endif

    uint16_t nx, ny;
    bool ok = atlas_insert(atlas, data, surface->w, surface->h, &nx, &ny);
#ifdef USE_EXTENDED_PALETTE
    omf_free(buf);
#endif
    if(ok) {
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
