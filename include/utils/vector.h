#ifndef VECTOR_H
#define VECTOR_H

#include "iterator.h"

typedef void (*vector_free_cb)(void *);
typedef int (*vector_compare_func)(const void *, const void *);

typedef struct vector_t {
    char *data;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int reserved;
    unsigned int inc_factor;
    vector_free_cb free_cb;
} vector;

void vector_create(vector *vector, unsigned int block_size);
void vector_create_cb(vector *vector, unsigned int block_size, vector_free_cb free_cb);
void vector_free(vector *vector);
void vector_clear(vector *vector);
void *vector_get(const vector *vector, unsigned int key);
int vector_append(vector *vector, const void *value);
int vector_prepend(vector *vector, const void *value);
void vector_sort(vector *vector, vector_compare_func cf);
unsigned int vector_size(const vector *vector);
int vector_delete(vector *vector, iterator *iterator);
void vector_iter_begin(const vector *vector, iterator *iter);
void vector_iter_end(const vector *vector, iterator *iter);

#endif // VECTOR_H
