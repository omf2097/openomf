#ifndef _ARRAY_H
#define _ARRAY_H

#include "iterator.h"

typedef struct array_t {
    unsigned int allocated_size;
    unsigned int filled;
    void **data;
} array;

void array_create(array *array);
void array_free(array *array);
void array_set(array *array, unsigned int key, const void *ptr);
void* array_get(array *array, unsigned int key);

void array_iter_begin(array *array, iterator *iterator);
void array_iter_end(array *arrat, iterator *iterator);

#endif // _ARRAY_H
