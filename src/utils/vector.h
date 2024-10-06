#ifndef VECTOR_H
#define VECTOR_H

#include "iterator.h"

typedef struct vector_t {
    char *data;
    unsigned int block_size;
    unsigned int blocks;
    unsigned int reserved;
} vector;

typedef int (*vector_compare_func)(const void *, const void *);

void vector_create(vector *vector, unsigned int block_size);
void vector_create_with_size(vector *vector, unsigned int block_size, unsigned int initial_size);
void vector_free(vector *vector);
void vector_clear(vector *vector);
void *vector_get(const vector *vector, unsigned int key);
int vector_set(vector *vector, unsigned int key, const void *value);
int vector_append(vector *vector, const void *value);
int vector_prepend(vector *vector, const void *value);
void vector_sort(vector *vector, vector_compare_func cf);
unsigned int vector_size(const vector *vector);
int vector_delete_at(vector *vec, int index);
int vector_delete(vector *vector, iterator *iterator);
void vector_pop(vector *vector);
void *vector_back(const vector *vector);
void vector_iter_begin(const vector *vector, iterator *iter);
void vector_iter_end(const vector *vector, iterator *iter);

#endif // VECTOR_H
