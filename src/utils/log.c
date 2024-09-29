#include "utils/log.h"
#include <stdarg.h>
#include <stdio.h>

FILE *handle = 0;
unsigned int _log_tick = 0;
char log_buf[1024];

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

void log_close(void) {
    if(handle != stdout && handle != 0) {
        fclose(handle);
    }
}

void log_hide(char mode, const char *fn, const char *fmt, ...) {
} // Do nothing here. This is a no-op logger.

void log_print(char mode, const char *fn, const char *fmt, ...) {
    if(handle == 0)
        return;
    int len;
    if(fn != NULL) {
        len = snprintf(log_buf, sizeof(log_buf), "[%7u][%c] %s(): ", _log_tick, mode, fn);
    } else {
        len = snprintf(log_buf, sizeof(log_buf), "[%7u][%c] ", _log_tick, mode);
    }
    va_list args;
    va_start(args, fmt);
    vsnprintf(&log_buf[len], sizeof(log_buf) - len, fmt, args);
    va_end(args);
    fprintf(handle, "%s\n", log_buf);
    fflush(handle);
}
