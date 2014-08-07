#ifndef _SD_ERROR_H
#define _SD_ERROR_H

#ifdef __cplusplus 
extern "C" {
#endif

#ifdef DEBUGMODE
    void debug_print(const char* fn, int line, const char *fmt, ...);
    #define DEBUGLOG(...) debug_print(__FUNCTION__, __LINE__, __VA_ARGS__)
#else
    #define DEBUGLOG(...)
#endif

enum {
    SD_SUCCESS,
    SD_FILE_OPEN_ERROR,
    SD_FILE_INVALID_TYPE,
    SD_FILE_PARSE_ERROR,
    SD_ANIM_INVALID_STRING,
    SD_OUT_OF_MEMORY,
    SD_INVALID_INPUT,
};

const char* sd_get_error(int errorcode);

#ifdef __cplusplus
}
#endif

#endif // _SD_ERROR_H
