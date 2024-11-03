#ifndef ALLOCATOR_DEFAULT_H
#define ALLOCATOR_DEFAULT_H

#include <stddef.h>

#define omf_free(ptr)                                                                                                  \
    do {                                                                                                               \
        free(ptr);                                                                                                     \
        (ptr) = NULL;                                                                                                  \
    } while(0)

static inline void *omf_malloc_real(size_t size, const char *file, int line) {
    void *ret = malloc(size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, _text_malloc_error, size, file, line);
    abort();
}

static inline void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
    void *ret = calloc(nmemb, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, _text_calloc_error, nmemb, size, file, line);
    abort();
}

static inline void *omf_realloc_real(void *ptr, size_t size, const char *file, int line) {
    void *ret = realloc(ptr, size);
    if(ret != NULL)
        return ret;
    fprintf(stderr, _text_realloc_error, ptr, size, file, line);
    abort();
}

#endif // ALLOCATOR_DEFAULT_H
