#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stddef.h>

typedef struct allocator_t {
    void  (*cfree)(void *ptr);
    void* (*crealloc)(void *ptr, size_t size);
} allocator;

#define omf_calloc(nmemb, size) \
    omf_calloc_real((nmemb), (size), __FILE__, __LINE__)
void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line);

#endif // _ALLOCATOR_H
