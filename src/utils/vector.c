#include "utils/vector.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include <stdlib.h>
#include <string.h>

void vector_init(vector *vec) {
    vec->blocks = 0;
    vec->free_cb = NULL;
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

void vector_create_cb(vector *vector, unsigned int block_size, vector_free_cb free_cb) {
    vector_create(vector, block_size);
    vector->free_cb = free_cb;
}

void vector_create_with_size(vector *vector, unsigned int block_size, unsigned int reserved) {
    vector->block_size = block_size;
    vector->reserved = reserved;
    vector_init(vector);
}

void vector_create_with_size_cb(vector *vector, unsigned int block_size, unsigned int reserved,
                                vector_free_cb free_cb) {
    vector_create_with_size(vector, block_size, reserved);
    vector->free_cb = free_cb;
}

void vector_clone(vector *dst, const vector *src) {
    dst->block_size = src->block_size;
    dst->blocks = src->blocks;
    dst->reserved = src->reserved;
    dst->free_cb = src->free_cb;
    size_t len = dst->reserved * dst->block_size;
    dst->data = (char *)omf_malloc(len);
    memcpy(dst->data, src->data, len);
}

void vector_clear(vector *vec) {
    if(vec->free_cb != NULL) {
        for(unsigned int i = 0; i < vec->blocks; i++) {
            vec->free_cb(vec->data + vec->block_size * i);
        }
    }
    vec->blocks = 0;
}

void vector_free(vector *vec) {
    vector_clear(vec);
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

void *vector_append_ptr(vector *vec) {
    if(vec->blocks >= vec->reserved) {
        vector_grow(vec);
    }
    void *dst = (char *)(vec->data + vec->blocks * vec->block_size);
    vec->blocks++;
    return dst;
}

void vector_append(vector *vec, const void *value) {
    memmove(vector_append_ptr(vec), value, vec->block_size);
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
    vec->blocks--;

    // Return success
    return 0;
}

int vector_swapdelete_at(vector *vec, unsigned index) {
    if(vec->blocks == 0)
        return 1;

    unsigned last = vec->blocks - 1;
    if(index != last) {
        memcpy(vector_get(vec, index), vector_get(vec, last), vec->block_size);
    }

    vec->blocks--;

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
    iter->peek = NULL;
    iter->prev = NULL;
    iter->ended = (vec->blocks == 0);
}

void vector_iter_end(const vector *vec, iterator *iter) {
    iter->data = vec;
    iter->vnow = NULL;
    iter->inow = vector_size(vec) - 1;
    iter->next = NULL;
    iter->peek = NULL;
    iter->prev = vector_iter_prev;
    iter->ended = (vec->blocks == 0);
}
