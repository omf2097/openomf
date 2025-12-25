#include <string.h>
#include <sys/stat.h>

#include "utils/io.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#if defined(_WIN32) || defined(WIN32)
#include <shlwapi.h> // PathFileExistsA
#else
#include <unistd.h> // access
#endif

// Anything above this seems to die on MSYS2 + mingw to EINVAL (22). Not sure why.
// Anyways, touch at your own peril.
#define READ_BLOCK_SIZE 1024

FILE *file_open(const char *file_name, const char *mode) {
    FILE *handle = fopen(file_name, mode);
    if(handle == NULL) {
        log_error("Unable to open file '%s'", file_name);
        return NULL;
    }
    return handle;
}

long file_size(FILE *handle) {
    struct stat info;
    fstat(fileno(handle), &info);
    return info.st_size;
}

bool file_read(FILE *handle, char *buffer, long size) {
    size_t ptr = 0;
    size_t read_size;
    while(1) {
        if(feof(handle)) {
            break;
        }
        if(ferror(handle)) {
            log_error("Error while reading file");
            return false;
        }
        read_size = min2(size - ptr, READ_BLOCK_SIZE);
        if(read_size <= 0) {
            break;
        }
        ptr += fread(buffer + ptr, 1, read_size, handle);
    }
    return true;
}

void file_close(FILE *handle) {
    fclose(handle);
}

bool file_exists(const char *file_name) {
#if defined(_WIN32) || defined(WIN32)
    return PathFileExistsA(file_name) == TRUE;
#else
    return access(file_name, F_OK) == 0;
#endif
}
