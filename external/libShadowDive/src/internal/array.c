#include "shadowdive/internal/array.h"
#include <stdlib.h>
#include <string.h>

void sd_array_create(void **mem, size_t item_size, int num_size) {
    *mem = calloc(num_size, item_size);
}

void sd_array_free(void **mem) {
    free(*mem);
    *mem = NULL;
}

void sd_array_resize(void **mem, size_t item_size, int num_size) {
    *mem = realloc(*mem, item_size * num_size);
}

void sd_array_delete(void *mem, size_t item_size, int *num_size, int index) {
    if(index == *num_size) {
        sd_array_pop(mem, item_size, num_size, NULL);
    } else {
        size_t off = item_size * index;
        size_t size = item_size * (*num_size - index);
        memmove((char*)mem + off, (char*)mem + off + item_size, size);
        (*num_size)--;
    }
}

void sd_array_insert(void *mem, size_t item_size, int *num_size, int index, void *new_item) {
    if(index == *num_size) {
        sd_array_push(mem, item_size, num_size, new_item);
    } else {
        size_t off = item_size * index;
        size_t size = item_size * (*num_size - index);
        memmove((char*)mem + off + item_size, (char*)mem + off, size);
        memcpy((char*)mem + off, new_item, item_size);
        (*num_size)++;
    }
}

void sd_array_push(void *mem, size_t item_size, int *num_size, void *new_item) {
    memcpy((char*)mem + item_size * (*num_size), new_item, item_size);
    (*num_size)++;
}

void sd_array_pop(void *mem, size_t item_size, int *num_size, void *old_item) {
    (*num_size)--;
    if(old_item != NULL) {
        memcpy(old_item, (char*)mem + item_size * (*num_size), item_size);
    }
}
