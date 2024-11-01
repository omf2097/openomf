#ifndef COMPAT_H
#define COMPAT_H

#include "platform.h"
#include <string.h>

#if __APPLE__
// MacOS X does not ship uchar.h
#include <stdint.h>
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
#else
#include <uchar.h>
#endif

#ifndef HAVE_STD_STRDUP
char *strdup(const char *s1);
#endif

#endif // COMPAT_H
