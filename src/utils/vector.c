#include "utils/vector.h"
#include <stdlib.h>
#include <string.h>

typedef struct vector_iterator_t {
    unsigned int now;
    unsigned int dir;
    vector *vec;
} vector_iterator;

void vector_init(vector *vec) {
    vec->blocks = 0;
    vec->reserved = 32;
    vec->inc_factor = 2;
    vec->data = (char*)vec->alloc.cmalloc(vec->reserved * vec->block_size);
}

void vector_create(vector *vec, unsigned int block_size) {
    vec->block_size = block_size;
    vec->alloc.cmalloc = malloc;
    vec->alloc.cfree = free;
    vec->alloc.crealloc = realloc;
    vector_init(vec);
}

void vector_create_with_allocator(vector *vec, unsigned int block_size, allocator alloc) {
    vec->block_size = block_size;
    vec->alloc = alloc;
    vector_init(vec);
}

void vector_free(vector *vec) {
    vec->blocks = 0;
    vec->reserved = 0;
    vec->block_size = 0;
    vec->alloc.cfree(vec->data);
}

void* vector_get(vector *vec, unsigned int key) {
    if(key >= vec->blocks) {
        return NULL;
    }
    return (char*)(vec->data + vec->block_size * key);
}

void vector_grow(vector *vec) {
    vec->data = vec->alloc.crealloc(vec->data, vec->reserved * vec->block_size * vec->inc_factor);
    vec->reserved = vec->reserved * vec->inc_factor;
}

void vector_append(vector *vec, const void *value) {
    if(vec->blocks >= vec->reserved) {
        vector_grow(vec);
    }
    void *dst = (char*)(vec->data + vec->blocks * vec->block_size);
    memcpy(dst, value, vec->block_size);
    vec->blocks++;
}

void vector_prepend(vector *vec, const void *value) {
    if(vec->blocks >= vec->reserved) {
        vector_grow(vec);
    }
    char *dst = (char*)(vec->data + vec->block_size);
    memmove(dst, vec->data, vec->block_size * vec->blocks);
    memcpy(dst, value, vec->block_size);
    vec->blocks++;
}

void vector_reserve(vector *vec, unsigned int blocks) {
    if(blocks > vec->reserved) {
        vec->data = vec->alloc.crealloc(vec->data, blocks * vec->block_size);
    }
}

unsigned int vector_size(vector *vec) {
    return vec->blocks;
}

void vector_delete(vector *vec, iterator *iter) {
    if(vec->blocks == 0) return;
    
    // Since last iteration already changed the "now" value, find the real "now" here.
    int real;
    if(iter->next == NULL) {
        real = iter->inow + 1;
    } else {
        real = iter->inow - 1;
    }
    
    // If this is NOT the last entry, we need to do memmove.
    if(real+1 < vec->blocks) {
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
}

void vector_sort(vector *vec, int (*compar)(const void*, const void*)) {
    qsort(vec->data, vec->blocks, vec->block_size, compar);
}

void* vector_iter_next(iterator *iter) {
    vector *vec = (vector*)iter->data;
    if(iter->inow + 1 >= vec->blocks) {
        iter->ended = 1;
    }
    void *addr = (void*)(vec->data + iter->inow * vec->block_size);
    iter->inow++;
    return addr;
}

void* vector_iter_prev(iterator *iter) {
    vector *vec = (vector*)iter->data;
    if(iter->inow == 0) {
        iter->ended = 1;
    }
    void *addr = (void*)(vec->data + iter->inow * vec->block_size);
    iter->inow--;
    return addr;
}

void vector_iter_begin(vector *vec, iterator *iter) {
    iter->data = vec;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = vector_iter_next;
    iter->prev = NULL;
    iter->ended = (vec->blocks == 0);
}

void vector_iter_end(vector *vec, iterator *iter) {
    iter->data = vec;
    iter->vnow = NULL;
    iter->inow = vector_size(vec)-1;
    iter->next = NULL;
    iter->prev = vector_iter_prev;
    iter->ended = (vec->blocks == 0);
}
