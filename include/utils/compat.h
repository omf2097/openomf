#ifndef _COMPAT_H
#define _COMPAT_H

#include "config.h"

#ifndef HAVE_STRDUP
char *strdup(const char *s1);
#endif

#endif // _COMPAT_H
