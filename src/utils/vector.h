#ifndef VECTOR_H
#define VECTOR_H

#include "iterator.h"

typedef void (*vector_free_cb)(void *);

typedef struct vector {
    char *data;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int reserved;
    vector_free_cb free_cb;
} vector;

typedef int (*vector_compare_func)(const void *, const void *);

void vector_create(vector *vector, unsigned int block_size);
void vector_create_cb(vector *vector, unsigned int block_size, vector_free_cb free_cb);
void vector_create_with_size(vector *vector, unsigned int block_size, unsigned int initial_size);
void vector_create_with_size_cb(vector *vector, unsigned int block_size, unsigned int initial_size,
                                vector_free_cb free_cb);
void vector_free(vector *vector);

void *vector_get(const vector *vector, unsigned int key);
int vector_set(vector *vector, unsigned int key, const void *value);
void vector_append(vector *vector, const void *value);
void *vector_append_ptr(vector *vec);

void vector_sort(vector *vector, vector_compare_func cf);

int vector_delete_at(vector *vec, unsigned index);
// removes an element by swapping it with the last element before popping.
int vector_swapdelete_at(vector *vec, unsigned index);
int vector_delete(vector *vector, iterator *iterator);

void vector_pop(vector *vector);
void *vector_back(const vector *vector);

void vector_iter_begin(const vector *vector, iterator *iter);
void vector_iter_end(const vector *vector, iterator *iter);

static inline unsigned int vector_size(const vector *vec) {
    return vec->blocks;
}

static inline void vector_clear(vector *vec) {
    vec->blocks = 0;
}

#endif // VECTOR_H
