#include "utils/array.h"
#include "utils/allocator.h"
#include <stdlib.h>

#define ARRAY_START_SIZE 64
#define PTR_SIZE(bs) ((bs) * sizeof(void *))

void array_nullify(void **ptr, unsigned int len) {
    for(int i = 0; i < len; i++) {
        ptr[i] = NULL;
    }
}

void array_create(array *array) {
    array->allocated_size = ARRAY_START_SIZE;
    array->filled = 0;
    array->data = omf_calloc(1, PTR_SIZE(array->allocated_size));
    array_nullify(array->data, array->allocated_size);
}

void array_free(array *array) {
    array->allocated_size = 0;
    array->filled = 0;
    omf_free(array->data);
}

void array_set(array *array, unsigned int key, const void *ptr) {
    if(key >= array->allocated_size) {
        unsigned int newsize = array->allocated_size * 2;
        if(key > newsize) {
            newsize = key;
        }
        array->data = omf_realloc(array->data, PTR_SIZE(newsize));
        array_nullify(array->data + array->allocated_size, newsize - array->allocated_size);
        array->allocated_size = newsize;
    }
    array->data[key] = (void *)ptr;
    array->filled += (ptr != NULL);
}

void *array_get(const array *array, unsigned int key) {
    if(key > array->allocated_size) {
        return NULL;
    }
    return array->data[key];
}

void *array_iter_next(iterator *iter) {
    array *arr = (array *)iter->data;
    void *out = NULL;
    while(iter->inow < arr->allocated_size) {
        if(arr->data[iter->inow] != NULL) {
            out = arr->data[iter->inow];
            iter->inow++;
            break;
        }
        iter->inow++;
    }
    if(iter->inow + 1 >= arr->allocated_size) {
        iter->ended = 1;
    }
    return out;
}

void *array_iter_prev(iterator *iter) {
    array *arr = (array *)iter->data;
    void *out = NULL;
    if(iter->inow == 0) {
        iter->ended = 1;
    }
    while(iter->inow >= 0) {
        if(arr->data[iter->inow] != NULL) {
            out = arr->data[iter->inow];
            iter->inow--;
            break;
        }
        iter->inow--;
    }
    return out;
}

void array_iter_begin(const array *array, iterator *iter) {
    iter->data = array;
    iter->vnow = NULL;
    iter->inow = 0;
    iter->next = array_iter_next;
    iter->prev = NULL;
    iter->ended = (array->filled == 0);
}

void array_iter_end(const array *array, iterator *iter) {
    iter->data = array;
    iter->vnow = NULL;
    iter->inow = array->allocated_size - 1;
    iter->next = NULL;
    iter->prev = array_iter_prev;
    iter->ended = (array->filled == 0);
}
