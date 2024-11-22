#include "utils/c_string_util.h"
#include "utils/allocator.h"
#include <string.h>

char *strncpy_or_truncate(char *dest, const char *src, size_t n) {
    char *ret = strncpy(dest, src, n);
    if(n > 0)
        dest[n - 1] = '\0';
    return ret;
}

char *omf_strdup_real(char const *s, char const *file, int line) {
    assert(s != NULL);
    size_t valid_range = strlen(s) + 1;
    char *new_s = omf_malloc_real(valid_range, file, line);
    memcpy(new_s, s, valid_range);
    return new_s;
}
