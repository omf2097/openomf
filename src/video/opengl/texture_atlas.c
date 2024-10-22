#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/hashmap.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "video/opengl/texture.h"
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
    GLuint tex_unit;
} texture_atlas;

int static inline zone_perimeter(zone *zone) {
    return zone->w * 2 + zone->h * 2;
}

texture_atlas *atlas_create(GLuint tex_unit, uint16_t width, uint16_t height) {
    texture_atlas *atlas = omf_calloc(1, sizeof(texture_atlas));
    hashmap_create(&atlas->items);
    vector_create(&atlas->free_space, sizeof(zone));
    atlas->w = width;
    atlas->h = height;
    atlas->tex_unit = tex_unit;
    atlas->texture_id = texture_create(tex_unit, width, height, GL_R8, GL_RED);
    zone item = {0, 0, width, height};
    vector_append(&atlas->free_space, &item);
    DEBUG("Texture atlas %dx%d created", width, height);
    return atlas;
}

void atlas_free(texture_atlas **atlas) {
    texture_atlas *obj = *atlas;
    if(obj != NULL) {
        hashmap_free(&obj->items);
        vector_free(&obj->free_space);
        texture_free(obj->tex_unit, obj->texture_id);
        omf_free(obj);
        *atlas = NULL;
        DEBUG("Texture atlas freed");
    }
}

/**
 * Finds the first free area that matches our minimum requirements.
 */
static bool find_free_space(const texture_atlas *atlas, uint16_t w, uint16_t h, int *got_index, zone *got_zone) {
    unsigned int vec_size = vector_size(&atlas->free_space);
    zone *best_item = NULL, *item;
    int seek_limit = 10;
    int best_index = -1, index;
    int best_perimeter = 4 * 4096 + 1, perimeter;
    for(int i = 0; i < vec_size; i++) {
        index = vec_size - i - 1;
        item = vector_get(&atlas->free_space, index);
        if(item->w >= w && item->h >= h) {
            perimeter = zone_perimeter(item);
            if(perimeter < best_perimeter) {
                best_index = index;
                best_perimeter = perimeter;
                best_item = item;
            }
        }
        if(best_index >= 0)
            seek_limit--;
        if(!seek_limit)
            break;
    }
    if(best_index == -1) {
        return false;
    }
    *got_index = best_index;
    memcpy(got_zone, best_item, sizeof(zone));
    return true;
}

/**
 * Split found space into smaller pieces.
 */
static void split_space(const zone *src, uint16_t w, uint16_t h, zone *a, zone *b) {
    uint16_t rw = src->w - w;
    uint16_t rh = src->h - h;
    if(rh > rw) {
        a->x = src->x + w;
        a->y = src->y;
        a->w = rw;
        a->h = h;
        b->x = src->x;
        b->y = src->y + h;
        b->w = src->w;
        b->h = rh;
    } else {
        a->x = src->x + w;
        a->y = src->y;
        a->w = rw;
        a->h = src->h;
        b->x = src->x;
        b->y = src->y + h;
        b->w = w;
        b->h = rh;
    }
}

static int space_sort(const void *a_ptr, const void *b_ptr) {
    zone *a = (zone *)a_ptr;
    zone *b = (zone *)b_ptr;
    int a_size = a->w + a->h;
    int b_size = b->w + b->h;
    return b_size - a_size;
}

bool atlas_insert(texture_atlas *atlas, const char *bytes, uint16_t w, uint16_t h, uint16_t *nx, uint16_t *ny) {
    zone free;
    int index;
    if(!find_free_space(atlas, w, h, &index, &free)) {
        PERROR("Texture atlas has no room for %dx%d area", w, h);
        return false;
    }

    // Try to split the found space into smaller areas, and add those to the end of the list.
    // Note that if the found area exactly matches the wanted one, we just clear out the found block.
    if(free.w == w && free.h == h) {
        vector_delete_at(&atlas->free_space, index);
    } else {
        zone split_a, split_b;
        split_space(&free, w, h, &split_a, &split_b);
        int size_a = split_a.w * split_a.h;
        int size_b = split_b.w * split_b.h;
        if(size_a == 0) {
            vector_set(&atlas->free_space, index, &split_b);
        } else if(size_b == 0) {
            vector_set(&atlas->free_space, index, &split_a);
        } else if(size_a < size_b) {
            vector_set(&atlas->free_space, index, &split_b);
            vector_append(&atlas->free_space, &split_a);
        } else {
            vector_set(&atlas->free_space, index, &split_a);
            vector_append(&atlas->free_space, &split_b);
        }
        vector_sort(&atlas->free_space, space_sort);
    }

    // Split found, add the area to the atlas.
    texture_update(atlas->tex_unit, atlas->texture_id, free.x, free.y, w, h, GL_RED, bytes);
    *nx = free.x;
    *ny = free.y;
    return true;
}

bool atlas_get(texture_atlas *atlas, const surface *surface, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h) {
    // First, check if item is already in the texture atlas. If it is, return coords immediately.
    zone *coords;
    if(hashmap_iget(&atlas->items, surface->guid, (void **)&coords, NULL) == 0) {
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
        zone cached = {nx, ny, surface->w, surface->h};
        hashmap_iput(&atlas->items, surface->guid, &cached, sizeof(zone));
        return true;
    }

    return false;
}

void atlas_reset(texture_atlas *atlas) {
    hashmap_clear(&atlas->items);
    vector_clear(&atlas->free_space);
    zone item = {0, 0, atlas->w, atlas->h};
    vector_append(&atlas->free_space, &item);
    INFO("Texture atlas reset");
}
