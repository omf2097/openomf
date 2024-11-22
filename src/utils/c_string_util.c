#include "utils/c_string_util.h"
#include <string.h>

char *strncpy_or_truncate(char *dest, const char *src, size_t n) {
    char *ret = strncpy(dest, src, n);
    if(n > 0)
        dest[n - 1] = '\0';
    return ret;
}
