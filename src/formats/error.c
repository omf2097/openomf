#include "formats/error.h"
#include <stdlib.h>

const char *sd_get_error(enum SD_ERRORCODE error_code) {
    switch(error_code) {
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
        case SD_FILE_WRITE_ERROR:
            return "File could not be written";
        case SD_FILE_READ_ERROR:
            return "File could not be read";
        case SD_FILE_UNLINK_ERROR:
            return "File could not be unlinked";
    }
    abort();
}
