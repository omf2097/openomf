#include <stdio.h>
#include <stdarg.h>
#include "utils/log.h"

FILE *handle = 0;
unsigned int _log_tick = 0;

int log_init(const char *filename) {
    if(handle)
        return 1;

    if(filename == 0) {
        handle = stdout;
    } else {
        handle = fopen(filename, "w");
        if(handle == NULL) {
            return 1;
        }
    }
    return 0;
}

void log_close() {
    if(handle != stdout && handle != 0) {
        fclose(handle);
    }
}

void log_print(char mode, const char *fn, const char *fmt, ...) {
    if(handle == 0)
        return;
    if(fn != NULL) {
        fprintf(handle, "[%7u][%c] %s(): ", _log_tick, mode, fn);
    } else {
        fprintf(handle, "[%7u][%c] ", _log_tick, mode);
    }
    va_list args;
    va_start(args, fmt);
    vfprintf(handle, fmt, args);
    va_end(args);
    fprintf(handle, "\n");
    fflush(handle);
}
