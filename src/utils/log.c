#include <stdio.h>
#include <stdarg.h>
#ifndef STANDALONE_SERVER
#include <SDL2/SDL.h>
#endif
#include "utils/log.h"

char log_msgbox_buffer[1024];
FILE *handle = 0;

int log_init(const char *filename) {
    if(handle) return 1;
    
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

void log_print(char mode, const char *fmt, ...) {
    fprintf(handle, "[%c] ", mode);
    va_list args;
    va_start(args, fmt);
    vfprintf(handle, fmt, args);
    va_end(args);
    fprintf(handle, "\n");
}

#ifndef STANDALONE_SERVER
void log_msgbox(char mode, const char *fmt, ...) {
    const char *modestr = "Message";
    switch(mode) {
        case 'E': modestr = "Error"; break;
        case 'I': modestr = "Info"; break;
        case 'D': modestr = "Debug"; break;
    }

    va_list args;
    va_start(args, fmt);
    vsnprintf(log_msgbox_buffer, sizeof(log_msgbox_buffer), fmt, args);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                             modestr, log_msgbox_buffer, NULL);
    va_end(args);
}
#endif

