#ifndef _ARRAY_H
#define _ARRAY_H

typedef struct array_t {
    unsigned int allocated_size;
    void **data;
} array;

typedef struct array_iterator_t {
    array *array;
    unsigned int now;
} array_iterator;

void array_create(array *array);
void array_free(array *array);
void array_insert(array *array, unsigned int key, void *ptr);
void* array_get(array *array, unsigned int key);

void  array_iter(array *array, array_iterator *iterator);
void* array_next(array_iterator *iterator);

#endif // _ARRAY_H