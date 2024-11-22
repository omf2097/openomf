#ifndef C_STRING_UTIL_H
#define C_STRING_UTIL_H

#include <stddef.h>

// strncpy() that guarantees null-termination.
char *strncpy_or_truncate(char *dest, const char *src, size_t n);

// strdup() that uses our allocator
char *omf_strdup_real(char const *s, char const *file, int line);
#define omf_strdup(s) omf_strdup_real((s), __FILE__, __LINE__)

#endif // C_STRING_UTIL_H
