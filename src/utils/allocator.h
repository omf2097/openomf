#ifndef ALLOCATOR_H
#define ALLOCATOR_H

// format strings for use in platform-specific allocator header
extern const char *_text_malloc_error;
extern const char *_text_calloc_error;
extern const char *_text_realloc_error;

// Add ifdefs here to include platform-specific allocators.
#include "utils/allocator_default.h"

/**
 * @brief Allocate a buffer
 * @details Allocates `size` bytes of un-initialized memory at an address
 * that is suitably aligned for mundane C types (pointers, structs, etc..).
 *
 * Passing a size of 0 is a constraints violation.
 *
 * @param size the number of bytes to allocate
 * @return the new allocation, a non-null pointer.
 */
#define omf_malloc(size) omf_malloc_real((size), __FILE__, __LINE__)

/**
 * @brief Allocate a zero-initialized buffer by element count
 * @details Allocates and zero-initializes enough memory for a array
 * of nmemb elements of size size.
 * Note that this may be larger than nmemb*size bytes of memory because
 * array elements might be padded to satisfy alignment.
 *
 * Passing a size or nmemb of 0 is a constraints violation.
 *
 * @param nmemb the number of elements to allocate space for
 * @param size the size of each element
 * @return the new allocation, a non-null pointer.
 */
#define omf_calloc(nmemb, size) omf_calloc_real((nmemb), (size), __FILE__, __LINE__)
/**
 * @brief Resize an allocation
 * @details Resize an allocation, either by expanding or shrinking the range of
 * addresses that can be validly used within the allocation (if there's room); or
 * by creating a new allocation of size size and copying as much of the old allocation's
 * contents as will fit in the new one, and freeing the old allocation.
 *
 * Passing a size or nmemb of 0 is a constraints violation.
 *
 * @param ptr the previous allocation if there was one, else NULL. This pointer is invalidated if omf_realloc succeeds.
 * @param size the number of bytes to allocate
 * @return the new allocation, a non-null pointer.
 */
#define omf_realloc(ptr, size) omf_realloc_real((ptr), (size), __FILE__, __LINE__);

/**
 * @brief Free an allocation by pointer, and reassigns the pointer to NULL.
 * @details Free an allocation from one of: omf_malloc, omf_calloc, or omf_realloc.
 * If the pointer is already NULL, does nothing (well-behaved).
 *
 * Do not pass pointers from other allocators (strdup, SDL_malloc, etc..), doing so triggers undefined behavior!
 * @param ptr the pointer to free & NULL.
 */
#define omf_free(ptr)                                                                                                  \
    do {                                                                                                               \
        omf_free_real(ptr);                                                                                            \
        (ptr) = NULL;                                                                                                  \
    } while(0)

#endif // ALLOCATOR_H
