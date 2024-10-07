#include "utils/allocator.h"
#include <stdio.h>
#include <stdlib.h>

void *omf_malloc_real(size_t size, const char *file, int line) {
    void *ret = malloc(size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "malloc(%zu) failed on %s:%d\n", size, file, line);
    abort();
}

void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
    void *ret = calloc(nmemb, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "calloc(%zu, %zu) failed on %s:%d\n", nmemb, size, file, line);
    abort();
}

void *omf_realloc_real(void *ptr, size_t size, const char *file, int line) {
    void *ret = realloc(ptr, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, "realloc(%p, %zu) failed on %s:%d\n", ptr, size, file, line);
    abort();
}
