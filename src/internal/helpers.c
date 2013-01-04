#include "shadowdive/internal/helpers.h"
#include <stdlib.h>

void alloc_or_realloc(void **ptr, int nsize) {
    if(*ptr) {
        realloc(*ptr, nsize);
    } else {
        *ptr = malloc(nsize+1);
    }
}