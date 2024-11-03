#ifndef ALLOCATOR_DEFAULT_H
#define ALLOCATOR_DEFAULT_H

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

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

static inline void *omf_alloc_with_options_real(size_t nmemb, size_t size, int options, const char *file, int line) {
    void *ret = NULL;
    if((options & ALLOC_HINT_TEXTURE) != 0) {
        assertf(size != 0, "texture size was 0");
        ret = malloc_uncached_aligned(64, size);
    } else {
        assert(false); // Unknown/unhandled option.
    }

    if(ret != NULL)
        return ret;
    fprintf(stderr, _text_calloc_error, nmemb, size, file, line);
    abort();
}

static inline void omf_free_with_options(void *ptr, int options) {
    if((options & ALLOC_HINT_TEXTURE) != 0) {
        free_uncached(ptr);
    } else {
        assert(false);
    }
}

#endif // ALLOCATOR_DEFAULT_H
