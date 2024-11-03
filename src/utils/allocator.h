#ifndef ALLOCATOR_H
#define ALLOCATOR_H

extern const char *_text_malloc_error;
extern const char *_text_calloc_error;
extern const char *_text_realloc_error;

// Add ifdefs here to include platform-specific allocators.
#include "utils/allocator_default.h"

#define omf_malloc(size) omf_malloc_real((size), __FILE__, __LINE__)
#define omf_calloc(nmemb, size) omf_calloc_real((nmemb), (size), __FILE__, __LINE__)
#define omf_realloc(ptr, size) omf_realloc_real((ptr), (size), __FILE__, __LINE__)

#endif // ALLOCATOR_H
