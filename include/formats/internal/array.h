#ifndef _SD_HELPERS_H
#define _SD_HELPERS_H

#include <stdlib.h>

void sd_array_create(void **mem, size_t item_size, int num_size);
void sd_array_free(void **mem);
void sd_array_resize(void **mem, size_t item_size, int num_size);
void sd_array_delete(void *mem, size_t item_size, int *num_size, int index);
void sd_array_insert(void *mem, size_t item_size, int *num_size, int index, void *new_item);
void sd_array_push(void *mem, size_t item_size, int *num_size, void *new_item);
void sd_array_pop(void *mem, size_t item_size, int *num_size, void *old_item);

#endif // _SD_HELPERS_H
