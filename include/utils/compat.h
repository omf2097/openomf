#ifndef _COMPAT_H
#define _COMPAT_H

#include <string.h>
#include "platform.h"

#ifndef HAVE_STD_STRDUP
char *strdup(const char *s1);
#endif

#endif // _COMPAT_H
