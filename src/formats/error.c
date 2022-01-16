#include "formats/error.h"
#include <string.h>

#ifdef DEBUGMODE

#include <stdarg.h>
#include <stdio.h>

void debug_print(const char *fn, int line, const char *fmt, ...) {
    printf("[%s:%d] ", fn, line);
    va_list args;
    va_start(args, fmt);
    printf(fmt, args);
    va_end(args);
    printf("\n");
}

#endif // DEBUGMODE

const char *sd_get_error(int errorcode) {
    switch(errorcode) {
        case SD_SUCCESS:
            return "No error";
        case SD_FILE_OPEN_ERROR:
            return "File does not exist";
        case SD_FILE_INVALID_TYPE:
            return "Corrupt file or invalid type";
        case SD_FILE_PARSE_ERROR:
            return "Parser error";
        case SD_ANIM_INVALID_STRING:
            return "Invalid animation string";
        case SD_OUT_OF_MEMORY:
            return "Out of memory";
        case SD_INVALID_INPUT:
            return "Invalid input/argument data";
        case SD_FORMAT_NOT_SUPPORTED:
            return "Format is not supported";
        case SD_INVALID_TAG:
            return "String contains an invalid tag";
        default:
            return "Unknown errorcode.";
    }
}
