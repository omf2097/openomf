#include "utils/c_string_util.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include <assert.h>
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

char *omf_strndup_real(char const *s, size_t n, char const *file, int line) {
    assert(s != NULL);
    size_t valid_range = smin2(strlen(s), n);
    char *new_s = omf_calloc_real(valid_range + 1, 1, file, line);
    memcpy(new_s, s, valid_range);
    return new_s;
}

size_t omf_strnlen_s(char const *str, size_t strsz) {
    assert(!(!str && strsz));
    if(!str) {
        return 0;
    }
    char const *found = memchr(str, '\0', strsz);
    return found ? (size_t)(found - str) : strsz;
}
