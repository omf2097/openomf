#include "shadowdive/error.h"
#include <string.h>

const char* sd_get_error(int errorcode) {
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
    default:
        return "Unknown errorcode.";
    }
}
