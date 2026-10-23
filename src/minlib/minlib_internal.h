#ifndef MINLIB_INTERNAL_H
#define MINLIB_INTERNAL_H

#include <stdarg.h>
#include <stdbool.h>

void sdl_set_error(const char *fmt, ...);
bool gl_minlib_load(void);

#endif /* MINLIB_INTERNAL_H */
