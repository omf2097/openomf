#include "utils/array.h"
#include <stdlib.h>
#include <string.h>

#define ARRAY_START_SIZE 64
#define PTR_SIZE(bs) ((bs) * sizeof(void*))

void array_nullify(void **ptr, unsigned int len) {
    for(int i = 0; i < len; i++) {
        ptr[i] = NULL;
    }
}

void array_create(array *array) {
    array->allocated_size = ARRAY_START_SIZE;
    array->data = malloc(PTR_SIZE(array->allocated_size));
    array_nullify(array->data, array->allocated_size);
}

void array_free(array *array) {
    array->allocated_size = 0;
    free(array->data);
}

void array_insert(array *array, unsigned int key, void *ptr) {
    if(key >= array->allocated_size) {
        unsigned int newsize = array->allocated_size * 2;
        if(key > newsize) {
            newsize = key;
        }
        array->data = realloc(array->data, PTR_SIZE(newsize));
        array_nullify(array->data + array->allocated_size, newsize - array->allocated_size);
        array->allocated_size = newsize;
    }
    array->data[key] = ptr;
}

void* array_get(array *array, unsigned int key) {
    if(key > array->allocated_size) {
        return 0;
    }
    return array->data[key];
}

void  array_iter(array *array, array_iterator *iterator) {
    iterator->array = array;
    iterator->now = 0;
}

void* array_next(array_iterator *iterator) {
    // Iterator is already at end, just return 0
    if(iterator->now >= iterator->array->allocated_size) {
        return 0;
    }
    
    // Walk through the array. This is pretty slow, since we need to check all slots ...
    unsigned int tmp;
    while(iterator->now < iterator->array->allocated_size) {
        tmp = iterator->now;
        if(iterator->array->data[tmp] != NULL) {
            iterator->now++;
            return iterator->array->data[tmp];
        }
        iterator->now++;
    }
    return 0;
}