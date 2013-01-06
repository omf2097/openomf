#ifndef _SD_ERROR_H
#define _SD_ERROR_H

#define SD_SUCCESS 0
#define SD_FILE_OPEN_ERROR 1
#define SD_FILE_INVALID_TYPE 2
#define SD_FILE_PARSE_ERROR 3
#define SD_ANIM_INVALID_STRING 4

void sd_get_error(char *message, int errorcode);

#endif // _SD_ERROR_H
