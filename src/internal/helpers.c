#include "shadowdive/internal/helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

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

char *sd_strdup(const char *s) {
    size_t len = strlen(s)+1;
    char *p = malloc(len);

    return p ? memcpy(p, s, len) : NULL;
}
