#include "shadowdive/internal/helpers.h"
#include <stdlib.h>
#include <stdio.h>

void alloc_or_realloc(void **ptr, int nsize) {
    if(*ptr) {
        void *newptr;
        if ((newptr = realloc(*ptr, nsize))) {
            // realloc didn't fail
            *ptr = newptr;
        } else {
            // realloc failed!
            perror("realloc failed");
            exit(1);
        }
    } else {
        *ptr = malloc(nsize+1);
    }
}
