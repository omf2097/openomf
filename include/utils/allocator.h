#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>

#define omf_calloc(nmemb, size) \
    omf_calloc_real((nmemb), (size), __FILE__, __LINE__)
void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line);

#define omf_realloc(ptr, size) \
    omf_realloc_real((ptr), (size), __FILE__, __LINE__)
void *omf_realloc_real(void *ptr, size_t size, const char *file, int line);

#define omf_free(ptr) do { \
    free(ptr);             \
    (ptr) = NULL;          \
} while(0)

#endif // _ALLOCATOR_H
