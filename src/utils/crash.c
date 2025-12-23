#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <utils/crash.h>

void _crash(const char *fmt, const char *function, const char *file, int line, ...) {
    va_list args;
    va_start(args, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, " @ %s(), %s:%d\n", function, file, line);
    va_end(args);
    fflush(stderr);
    abort();
}
