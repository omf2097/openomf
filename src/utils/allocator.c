#include "utils/allocator.h"

const char *_text_malloc_error = "malloc(%zu) failed on %s:%d\n";
const char *_text_calloc_error = "calloc(%zu, %zu) failed on %s:%d\n";
const char *_text_realloc_error = "realloc(%p, %zu) failed on %s:%d\n";
