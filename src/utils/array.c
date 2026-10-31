#include "utils/array.h"
#include "utils/allocator.h"
#include <stdlib.h>

#define ARRAY_START_SIZE 64
#define PTR_SIZE(bs) ((bs) * sizeof(void *))

void array_nullify(void **ptr, unsigned int len) {
    for(unsigned i = 0; i < len; i++) {
        ptr[i] = NULL;
    }
}

void array_create_with_size(array *array, const unsigned int initial_size) {
    array->allocated_size = initial_size;
    array->filled = 0;
    array->data = omf_calloc(1, PTR_SIZE(initial_size));
    array->free_cb = NULL;
    array_nullify(array->data, initial_size);
}

void array_create(array *array) {
    array_create_with_size(array, ARRAY_START_SIZE);
}

void array_create_cb(array *array, const array_free_cb free_cb) {
    array_create(array);
    array->free_cb = free_cb;
}

void array_create_with_size_cb(array *array, unsigned int initial_size, array_free_cb free_cb) {
    array_create_with_size(array, initial_size);
    array->free_cb = free_cb;
}

void array_free(array *array) {
    if(array->free_cb != NULL) {
        for(unsigned int i = 0; i < array->allocated_size; i++) {
            if(array->data[i] != NULL) {
                array->free_cb(array->data[i]);
                array->data[i] = NULL;
            }
        }
    }
    array->allocated_size = 0;
    array->filled = 0;
    array->free_cb = NULL;
    omf_free(array->data);
}

void array_set(array *array, const unsigned int key, const void *ptr) {
    if(key >= array->allocated_size) {
        unsigned int new_size = array->allocated_size * 2;
        if(key >= new_size) {
            new_size = key + 1;
        }
        array->data = omf_realloc(array->data, PTR_SIZE(new_size));
        array_nullify(array->data + array->allocated_size, new_size - array->allocated_size);
        array->allocated_size = new_size;
    }
    if(array->data[key] != NULL) {
        array->filled--;
    }
    array->data[key] = (void *)ptr;
    if(ptr != NULL) {
        array->filled++;
    }
}

void *array_get(const array *array, const unsigned int key) {
    if(key >= array->allocated_size) {
        return NULL;
    }
    return array->data[key];
}

int array_delete_at(array *array, const unsigned int key) {
    if(key >= array->allocated_size || array->data[key] == NULL) {
        return 1;
    }
    if(array->free_cb != NULL) {
        array->free_cb(array->data[key]);
    }
    array->data[key] = NULL;
    array->filled--;
    return 0;
}

void *array_iter_next(iterator *iter) {
    const array *arr = (array *)iter->data;
    void *out = NULL;
    while(iter->inow < (int)arr->allocated_size) {
        if(arr->data[iter->inow] != NULL) {
            out = arr->data[iter->inow];
            iter->inow++;
            break;
        }
        iter->inow++;
    }
    if(iter->inow >= (int)arr->allocated_size) {
        iter->ended = 1;
    }
    return out;
}

void *array_iter_prev(iterator *iter) {
    const array *arr = (array *)iter->data;
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
    iter->peek = NULL;
    iter->prev = NULL;
    iter->ended = (array->filled == 0);
}

void array_iter_end(const array *array, iterator *iter) {
    iter->data = array;
    iter->vnow = NULL;
    iter->inow = array->allocated_size - 1;
    iter->next = NULL;
    iter->peek = NULL;
    iter->prev = array_iter_prev;
    iter->ended = (array->filled == 0);
}
