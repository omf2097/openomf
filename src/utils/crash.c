#include <stdio.h>
#include <stdlib.h>

#include <utils/crash.h>

void _crash(const char *message, const char *function, const char *file, int line) {
    fprintf(stderr, "%s @ %s(), %s:%d\n", message, function, file, line);
    fflush(stderr);
    abort();
}
