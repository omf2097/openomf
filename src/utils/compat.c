#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/compat.h"

#ifndef HAVE_STD_STRDUP
char* strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *d = omf_calloc(len, 1);
    if(d != NULL)
        strncpy(d, s, len);
    return d;
}
#endif
