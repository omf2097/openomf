#include "formats/error.h"

const char *sd_get_error(int error_code) {
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
        default:
            return "Unknown errorcode.";
    }
}
