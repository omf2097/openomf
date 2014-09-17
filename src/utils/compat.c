#include <stdlib.h>

#include "utils/compat.h"

#ifndef HAVE_STRDUP
char* strdup(const char *s) {
    char *d = malloc(strlen(s) + 1);
    if(d != NULL)
        strcpy(d, s);
    return d;
}
#endif
