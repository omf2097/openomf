#ifndef _VECTOR_H
#define _VECTOR_H

#include "iterator.h"
#include "allocator.h"

typedef struct vector_t {
    char *data;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int reserved;
    unsigned int inc_factor;
    allocator alloc;
} vector;

typedef int (*vector_compare_func)(const void*, const void*);

void vector_create_with_allocator(vector *vector, unsigned int block_size, allocator alloc);
void vector_create(vector *vector, unsigned int block_size);
void vector_free(vector *vector);
void vector_clear(vector *vector);
void* vector_get(vector *vector, unsigned int key);
int vector_append(vector *vector, const void *value);
int vector_prepend(vector *vector, const void *value);
void vector_sort(vector *vector, vector_compare_func cf);
unsigned int vector_size(vector *vector);
void vector_clear(vector *vec);
int vector_delete(vector *vector, iterator *iterator);
void vector_iter_begin(vector *vector, iterator *iter);
void vector_iter_end(vector *vector, iterator *iter);

#endif // _VECTOR_H
