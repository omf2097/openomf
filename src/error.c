#include "shadowdive/error.h"
#include <string.h>

void sd_get_error(char *message, int errorcode) {
    switch(errorcode) {
    case SD_SUCCESS:
        strcpy(message, "No error");
        break;
    case SD_FILE_OPEN_ERROR:
        strcpy(message, "File does not exist");
        break;
    case SD_FILE_INVALID_TYPE:
        strcpy(message, "Corrupt file or invalid type");
        break;
    case SD_FILE_PARSE_ERROR:
        strcpy(message, "Parser error");
        break;
    case SD_ANIM_INVALID_STRING:
        strcpy(message, "Invalid animation string");
        break;
    }
}
