#include "utils/allocator.h"
#include <stdlib.h>
#include <stdio.h>

void *omf_calloc_real(size_t nmemb, size_t size, const char *file, int line) {
    void *ret = calloc(nmemb, size);
    if (ret != NULL)
        return ret;
    fprintf(stderr, "calloc(%zu, %zu) failed on %s:%d\n",
            nmemb, size, file, line);
    abort();
}
