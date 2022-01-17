#ifndef COMPAT_H
#define COMPAT_H

#include "platform.h"
#include <string.h>

#ifndef HAVE_STD_STRDUP
char *strdup(const char *s1);
#endif

#endif // COMPAT_H
