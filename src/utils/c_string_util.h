#ifndef C_STRING_UTIL_H
#define C_STRING_UTIL_H

#include <stddef.h>

// strncpy() that guarantees null-termination.
char *strncpy_or_truncate(char *dest, const char *src, size_t n);

#endif // C_STRING_UTIL_H
