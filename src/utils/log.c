#include "utils/log.h"
#include <stdio.h>
#include <stdarg.h>

FILE *handle = 0;

int log_init(const char *filename) {
    if(handle) return 1;
    
    if(filename == 0) {
        handle = stdout;
    } else {
        handle = fopen(filename, "w");
    }
    return 0;
}

void log_close() {
    if(handle != stdout && handle != 0) {
        fclose(handle);
    }
}

void log_print(char mode, char *fmt, ...) {
    fprintf(handle, "[%c] ", mode);
    va_list args;
    va_start(args, fmt);
    vfprintf(handle, fmt, args);
    va_end(args);
    fprintf(handle, "\n");
}