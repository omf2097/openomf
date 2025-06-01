#include <assert.h>
#include <stdlib.h>

#include "utils/sprite_atlas.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/vector.h"

typedef struct atlas_item {
    rect16 area;
    atlas_index parent;
    bool in_use;
    bool is_leaf;
} atlas_item;

typedef struct available_item {
    atlas_index index;
    rect16 area;
} available_item;

typedef struct sprite_atlas {
    vector tree;
    vector free_space;
    uint16_t w;
    uint16_t h;
} sprite_atlas;

sprite_atlas *atlas_create(uint16_t width, uint16_t height) {
    sprite_atlas *atlas = omf_calloc(1, sizeof(sprite_atlas));
    vector_create_with_size(&atlas->tree, sizeof(atlas_item), 121);
    vector_create_with_size(&atlas->free_space, sizeof(available_item), 32);
    available_item *leaf_item = vector_append_ptr(&atlas->free_space);
    *leaf_item = (available_item){0, {0, 0, width, height}};
    atlas_item *tree_item = vector_append_ptr(&atlas->tree);
    *tree_item = (atlas_item){{0, 0, width, height}, -1, false, true};
    atlas->w = width;
    atlas->h = height;
    return atlas;
}

void atlas_free(sprite_atlas **atlas) {
    if(atlas != NULL && (*atlas) != NULL) {
        vector_free(&(*atlas)->tree);
        vector_free(&(*atlas)->free_space);
        omf_free(*atlas);
    }
}

static inline int32_t perimeter(rect16 *area) {
    return area->w * 2 + area->h * 2;
}

/**
 * Finds the first free area that matches our minimum requirements.
 */
static bool find_free_space(const sprite_atlas *atlas, uint16_t w, uint16_t h, atlas_index *best_index, size_t *best_item) {
    iterator it;
    vector_iter_begin(&atlas->free_space, &it);
    available_item *item;
    size_t current_item = 0;
    int32_t best_perimeter = INT32_MAX;
    *best_index = -1;
    log_info("Search");
    foreach(it, item) {
        int32_t p = perimeter(&item->area);
        log_info("Item perimeter = %d", p);
        if(item->area.w == w && item->area.h == h) {
            // Don't bother continuing.
            *best_index = item->index;
            *best_item = current_item;
            log_info("Found exact match!");
            break;
        }
        if(item->area.w >= w && item->area.h >= h && p < best_perimeter) {
            *best_index = item->index;
            *best_item = current_item;
            best_perimeter = p;
            break;
        }
        current_item++;
    }
    log_info("Best index = %d", *best_index);
    return *best_index > -1;
}

/**
 * Split given area into smaller pieces.
 * @param src Area to split
 * @param w Width to reserve for piece a
 * @param h Height to reserve for piece a
 * @param a Reserved piece
 * @param b Leftover piece 1
 * @param c Leftover piece 2
 */
static void split_space(const rect16 *src, uint16_t w, uint16_t h, rect16 *a, rect16 *b, rect16 *c) {
    *a = (rect16){src->x, src->y, w, h};
    *b = (rect16){src->x + w, src->y, src->w - w, h};
    *c = (rect16){src->x, src->y + h, src->w, src->h - h};
}

/**
 * Sort function for free space vector.
 */
static int space_sort(const void *a_ptr, const void *b_ptr) {
    available_item *a = (available_item *)a_ptr;
    available_item *b = (available_item *)b_ptr;
    int a_size = a->area.w + a->area.h;
    int b_size = b->area.w + b->area.h;
    return a_size - b_size;
}

atlas_index atlas_insert(sprite_atlas *atlas, uint16_t w, uint16_t h, rect16 *item) {
    size_t free_item_index;
    atlas_index parent_index;
    if(!find_free_space(atlas, w, h, &parent_index, &free_item_index)) {
        return -1;
    }

    // This is the actual space we are going to use (and split).
    atlas_item *parent = vector_get(&atlas->tree, parent_index);
    parent->is_leaf = false; // This is now a node.

    // Create the new leaf items and link them to the parent. First item is always the reserved sprite.
    size_t index_a, index_b, index_c;
    atlas_item *child_a = vector_append_ptr_index(&atlas->tree, &index_a);
    atlas_item *child_b = vector_append_ptr_index(&atlas->tree, &index_b);
    atlas_item *child_c = vector_append_ptr_index(&atlas->tree, &index_c);
    child_a->is_leaf = child_b->is_leaf = child_c->is_leaf = true;
    child_a->in_use = child_b->in_use = child_c->in_use = true;
    child_a->parent = child_b->parent = child_c->parent = parent_index;

    // Split. A -> reserved sprite, B -> remainder right, C -> remainder top
    split_space(&parent->area, w, h, &child_a->area, &child_b->area, &child_b->area);

    // Reuse the old position in free space vector (quicksort will move it)
    available_item *free_space_a = vector_get(&atlas->free_space, free_item_index);
    free_space_a->area = child_b->area;
    free_space_a->index = index_b;

    // Append the last new item to the end (again, quicksort will move it)
    available_item *free_space_b = vector_append_ptr(&atlas->free_space);
    free_space_b->area = child_c->area;
    free_space_b->index = index_c;

    // Keep the free space items sorted so that we can quickly binary-search
    vector_sort(&atlas->free_space, space_sort);

    // That's that.
    *item = child_a->area;
    return index_a;
}

bool atlas_get(const sprite_atlas *atlas, atlas_index key, rect16 *item) {
    atlas_item *coords = vector_get(&atlas->tree, key);
    if(coords != NULL) {
        *item = coords->area;
        return true;
    }
    return false;
}

