#include "utils/msgbox.h"
#include <SDL.h>
#include <stdarg.h>
#include <stdio.h>

static char err_msgbox_buffer[1024];

void err_msgbox(const char *fmt, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    vsnprintf(err_msgbox_buffer, sizeof(err_msgbox_buffer), fmt, args);
    va_end(args);
    if(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", err_msgbox_buffer, NULL) != 0) {
        // if the message box failed, fallback to fprintf
        vfprintf(stderr, fmt, args_copy);
        fprintf(stderr, "... Exiting\n");
    }
    va_end(args_copy);
}
