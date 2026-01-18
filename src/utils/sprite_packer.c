/**
 * @file sprite_packer.c
 * @brief Rectangle packing algorithm implementation.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#include "utils/sprite_packer.h"
#include "utils/allocator.h"
#include "utils/vector.h"

#define SEEK_lIMIT 10

struct sprite_packer {
    vector free_space;
    uint16_t w;
    uint16_t h;
};

/**
 * @brief Calculate the perimeter of a region.
 * @param region Region to measure
 * @return Perimeter in pixels
 */
static inline int region_perimeter(const sprite_region *region) {
    return region->w * 2 + region->h * 2;
}

/**
 * @brief Find the first free area that matches minimum requirements.
 * @details Uses a best-fit strategy, looking for the smallest region that fits.
 *          Searches backwards through the free space list with a seek limit.
 * @param packer The packer to search
 * @param w Minimum width required
 * @param h Minimum height required
 * @param got_index Output: index of the found region in the free space list
 * @param got_region Output: copy of the found region
 * @return true if a suitable region was found, false otherwise
 */
static bool find_free_space(const sprite_packer *packer, uint16_t w, uint16_t h, int *got_index,
                            sprite_region *got_region) {
    const unsigned int vec_size = vector_size(&packer->free_space);
    const sprite_region *best_item = NULL;
    int seek_limit = SEEK_lIMIT;
    int best_index = -1;
    int best_perimeter = 2 * packer->w + 2 * packer->h + 1;
    for(int i = 0; i < (int)vec_size; i++) {
        const int index = vec_size - i - 1;
        const sprite_region *item = vector_get(&packer->free_space, index);
        if(item->w >= w && item->h >= h) {
            const int perimeter = region_perimeter(item);
            if(perimeter < best_perimeter) {
                best_index = index;
                best_perimeter = perimeter;
                best_item = item;
            }
        }
        if(best_index >= 0) {
            seek_limit--;
        }
        if(!seek_limit) {
            break;
        }
    }
    if(best_index == -1 || best_item == NULL) {
        return false;
    }
    *got_index = best_index;
    *got_region = *best_item;
    return true;
}

/**
 * @brief Split a region into two smaller pieces after allocating from it.
 * @details Splits along the longer remaining dimension to minimize fragmentation.
 *          Region 'a' is placed to the right of the allocated area, region 'b' below.
 * @param src Source region to split
 * @param w Width of the allocated area
 * @param h Height of the allocated area
 * @param a Output: first remaining region (right side)
 * @param b Output: second remaining region (bottom)
 */
static void split_space(const sprite_region *src, uint16_t w, uint16_t h, sprite_region *a, sprite_region *b) {
    const uint16_t rw = src->w - w;
    const uint16_t rh = src->h - h;
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

/**
 * @brief Comparison function for sorting regions by size (largest first).
 * @details Used with vector_sort to keep larger regions at the front.
 */
static int space_sort(const void *a_ptr, const void *b_ptr) {
    const sprite_region *a = a_ptr;
    const sprite_region *b = b_ptr;
    const int a_size = a->w + a->h;
    const int b_size = b->w + b->h;
    return b_size - a_size;
}

sprite_packer *sprite_packer_create(uint16_t width, uint16_t height) {
    sprite_packer *packer = omf_calloc(1, sizeof(sprite_packer));
    vector_create(&packer->free_space, sizeof(sprite_region));
    packer->w = width;
    packer->h = height;
    const sprite_region item = {0, 0, width, height};
    vector_append(&packer->free_space, &item);
    return packer;
}

void sprite_packer_free(sprite_packer **packer) {
    sprite_packer *obj = *packer;
    if(obj != NULL) {
        vector_free(&obj->free_space);
        omf_free(obj);
        *packer = NULL;
    }
}

bool sprite_packer_alloc(sprite_packer *packer, uint16_t w, uint16_t h, sprite_region *out) {
    sprite_region free_region;
    int index;
    if(!find_free_space(packer, w, h, &index, &free_region)) {
        return false;
    }

    // Try to split the found space into smaller areas, and add those to the end of the list.
    // Note that if the found area exactly matches the wanted one, we just clear out the found block.
    if(free_region.w == w && free_region.h == h) {
        vector_delete_at(&packer->free_space, index);
    } else {
        sprite_region split_a, split_b;
        split_space(&free_region, w, h, &split_a, &split_b);
        const int size_a = split_a.w * split_a.h;
        const int size_b = split_b.w * split_b.h;
        if(size_a == 0) {
            vector_set(&packer->free_space, index, &split_b);
        } else if(size_b == 0) {
            vector_set(&packer->free_space, index, &split_a);
        } else if(size_a < size_b) {
            vector_set(&packer->free_space, index, &split_b);
            vector_append(&packer->free_space, &split_a);
        } else {
            vector_set(&packer->free_space, index, &split_a);
            vector_append(&packer->free_space, &split_b);
        }
        vector_sort(&packer->free_space, space_sort);
    }

    out->x = free_region.x;
    out->y = free_region.y;
    out->w = w;
    out->h = h;
    return true;
}

void sprite_packer_reset(sprite_packer *packer) {
    vector_clear(&packer->free_space);
    const sprite_region item = {0, 0, packer->w, packer->h};
    vector_append(&packer->free_space, &item);
}

uint16_t sprite_packer_get_width(const sprite_packer *packer) {
    return packer->w;
}

uint16_t sprite_packer_get_height(const sprite_packer *packer) {
    return packer->h;
}
