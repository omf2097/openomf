#include "utils/vector.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include <stdlib.h>
#include <string.h>

void vector_init(vector *vec) {
    vec->blocks = 0;
    if(vec->reserved) {
        vec->data = (char *)omf_malloc(vec->reserved * vec->block_size);
    } else {
        vec->data = NULL;
    }
}

void vector_create(vector *vec, unsigned int block_size) {
    vec->block_size = block_size;
    vec->reserved = 32;
    vector_init(vec);
}

void vector_create_with_size(vector *vec, unsigned int block_size, unsigned int reserved) {
    vec->block_size = block_size;
    vec->reserved = reserved;
    vector_init(vec);
}

void vector_clear(vector *vec) {
    vec->blocks = 0;
}

void vector_free(vector *vec) {
    vec->blocks = 0;
    vec->reserved = 0;
    vec->block_size = 0;
    omf_free(vec->data);
}

void *vector_get(const vector *vec, unsigned int key) {
    if(key >= vec->blocks) {
        return NULL;
    }
    return (char *)(vec->data + vec->block_size * key);
}

void *vector_back(const vector *vec) {
    if(vec->blocks <= 0) {
        return NULL;
    }
    return (char *)(vec->data + vec->block_size * (vec->blocks - 1));
}

int vector_set(vector *vec, unsigned int key, const void *value) {
    if(key >= vec->blocks)
        return 1;
    void *dst = (char *)(vec->data + key * vec->block_size);
    memmove(dst, value, vec->block_size);
    return 0;
}

static void vector_grow(vector *vec) {
    int current_size = max2(1, vec->reserved);
    int new_size = current_size + max2(1, (current_size >> 2));
    vec->data = omf_realloc(vec->data, new_size * vec->block_size);
    vec->reserved = new_size;
}

int vector_append(vector *vec, const void *value) {
    if(vec->blocks >= vec->reserved) {
        vector_grow(vec);
    }
    void *dst = (char *)(vec->data + vec->blocks * vec->block_size);
    memmove(dst, value, vec->block_size);
    vec->blocks++;
    return 0;
}

int vector_prepend(vector *vec, const void *value) {
    if(vec->blocks >= vec->reserved) {
        vector_grow(vec);
    }
    char *dst = (char *)(vec->data + vec->block_size);
    memmove(dst, vec->data, vec->block_size * vec->blocks);
    memcpy(dst, value, vec->block_size);
    vec->blocks++;
    return 0;
}

unsigned int vector_size(const vector *vec) {
    return vec->blocks;
}

void vector_pop(vector *vec) {
    if(vec->blocks > 0) {
        vec->blocks--;
    }
}

int vector_delete_at(vector *vec, unsigned index) {
    if(vec->blocks == 0)
        return 1;

    // If this is NOT the last entry, we need to do memmove.
    if(index + 1 < vec->blocks) {
        void *dst = vec->data + index * vec->block_size;
        void *src = vec->data + (index + 1) * vec->block_size;
        unsigned int size = (vec->blocks - 1 - index) * vec->block_size;
        memmove(dst, src, size);
    }

    // We deleted an entry, so blocks-1
    if(vec->blocks > 0) {
        vec->blocks--;
    }

    // Return success
    return 0;
}

int vector_delete(vector *vec, iterator *iter) {
    if(vec->blocks == 0)
        return 1;

    // Since last iteration already changed the "now" value, find the real "now" here.
    int real;
    if(iter->next == NULL) {
        real = iter->inow + 1;
    } else {
        real = iter->inow - 1;
    }

    // If this is NOT the last entry, we need to do memmove.
    if(real + 1 < (int)vec->blocks) {
        void *dst = vec->data + real * vec->block_size;
        void *src = vec->data + (real + 1) * vec->block_size;
        unsigned int size = (vec->blocks - 1 - real) * vec->block_size;
        memmove(dst, src, size);

        // If we are iterating forwards, moving an entry will hop iterator forwards by two.
        // We will fix this issue by hopping backwards by one.
        if(iter->prev == NULL) {
            iter->inow--;
        }
    }

    // We deleted an entry, so blocks-1
    if(vec->blocks > 0) {
        vec->blocks--;
    }

    // Return success
    return 0;
}

void vector_sort(vector *vec, vector_compare_func cf) {
    qsort(vec->data, vec->blocks, vec->block_size, cf);
}

void *vector_iter_next(iterator *iter) {
    vector *vec = (vector *)iter->data;
    if(iter->inow + 1 >= (int)vec->blocks) {
        iter->ended = 1;
    }
    void *addr = (void *)(vec->data + iter->inow * vec->block_size);
    iter->inow++;
    return addr;
}

void *vector_iter_prev(iterator *iter) {
    vector *vec = (vector *)iter->data;
    if(iter->inow == 0) {
        iter->ended = 1;
    }
    void *addr = (void *)(vec->data + iter->inow * vec->block_size);
    iter->inow--;
    return addr;
}

void vector_iter_begin(const vector *vec, iterator *iter) {
    iter->data = vec;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = vector_iter_next;
    iter->prev = NULL;
    iter->ended = (vec->blocks == 0);
}

void vector_iter_end(const vector *vec, iterator *iter) {
    iter->data = vec;
    iter->vnow = NULL;
    iter->inow = vector_size(vec) - 1;
    iter->next = NULL;
    iter->prev = vector_iter_prev;
    iter->ended = (vec->blocks == 0);
}
