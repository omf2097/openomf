#ifndef C_STRING_UTIL_H
#define C_STRING_UTIL_H

#include <stddef.h>

// strncpy() that guarantees null-termination.
char *strncpy_or_truncate(char *dest, const char *src, size_t n);

// strdup() that uses our allocator
char *omf_strdup_real(char const *s, char const *file, int line);
#define omf_strdup(s) omf_strdup_real((s), __FILE__, __LINE__)

// Reads up to strsz bytes of str, returning the position of the
// first null character or strsz if it was not found.
size_t omf_strnlen_s(char const *str, size_t strsz);

#endif // C_STRING_UTIL_H
