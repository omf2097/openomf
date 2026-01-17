/**
 * @file allocator_default.h
 * @brief Default memory allocator implementation using standard library functions.
 * @details This is the default allocator backend that wraps standard malloc/calloc/realloc/free.
 *          On allocation failure, these functions crash with an error message instead of
 *          returning NULL.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef ALLOCATOR_DEFAULT_H
#define ALLOCATOR_DEFAULT_H

#include "utils/crash.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/** @internal */
#define omf_free_real(ptr) free(ptr)

/**
 * @internal
 * @brief Internal implementation - use omf_malloc() macro instead.
 * @see omf_malloc
 */
static inline void *omf_malloc_real(size_t size, const char *file, int line) {
    assert(size > 0);
    void *ret = malloc(size);
    if(ret != NULL) {
        return ret;
    }
    crash_with_args(_text_malloc_error, size);
}

/**
 * @internal
 * @brief Internal implementation - use omf_calloc() macro instead.
 * @see omf_calloc
 */
static inline void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
    assert(size > 0);
    assert(nmemb > 0);
    void *ret = calloc(nmemb, size);
    if(ret != NULL) {
        return ret;
    }
    crash_with_args(_text_calloc_error, nmemb, size);
}

/**
 * @internal
 * @brief Internal implementation - use omf_realloc() macro instead.
 * @see omf_realloc
 */
static inline void *omf_realloc_real(void *ptr, size_t size, const char *file, int line) {
    assert(size > 0);
    void *ret = realloc(ptr, size);
    if(ret != NULL) {
        return ret;
    }
    crash_with_args(_text_realloc_error, ptr, size);
}

#endif // ALLOCATOR_DEFAULT_H
