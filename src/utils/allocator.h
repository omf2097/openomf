#ifndef ALLOCATOR_H
#define ALLOCATOR_H

extern const char *_text_malloc_error;
extern const char *_text_calloc_error;
extern const char *_text_realloc_error;

// Allocator options
#define ALLOC_HINT_TEXTURE 0x00000001

// Add ifdefs here to include platform-specific allocators.
#ifdef N64_BUILD
#include "utils/allocator_n64.h"
#else
#include "utils/allocator_default.h"
#endif

#define omf_malloc(size) omf_malloc_real((size), __FILE__, __LINE__)
#define omf_calloc(nmemb, size) omf_calloc_real((nmemb), (size), __FILE__, __LINE__)
#define omf_realloc(ptr, size) omf_realloc_real((ptr), (size), __FILE__, __LINE__)

// omf_alloc_with_options, this function allocates memory that is to be shared between the engine (on the CPU) and the
// hardware directly. These allocations can be textures, palettes, audio buffers or pointers to read only memory.
// The options flags are hints as to what type of allocation is necessary, it is up to the platform's implementation to
// decide how to translate the flags: what alignment is necessary to be optimal and whether memory needs to be allocated
// cached or uncached.
// Using this function can allow the platform to save on copies and translations in the platform hardware implementation
// layers.
//
// NOTE: An allocation with a specific set of options also requires a matching free with the same set of options.
//
#define omf_alloc_with_options(nmemb, size, options)                                                                   \
    omf_alloc_with_options_real((nmemb), (size), (options), __FILE__, __LINE__)
#endif // ALLOCATOR_H
