#include <string.h>
#include <sys/stat.h>

#include "utils/io.h"
#include "utils/log.h"
#include "utils/miscmath.h"

#define READ_BLOCK_SIZE 32

FILE *file_open(const char *file_name, const char *mode) {
    FILE *handle = fopen(file_name, mode);
    if(handle == NULL) {
        PERROR("Unable to open file '%s'", file_name);
        abort();
    }
    return handle;
}

long file_size(FILE *handle) {
    struct stat info;
    fstat(fileno(handle), &info);
    return info.st_size;
}

void file_read(FILE *handle, char *buffer, long size) {
    size_t ptr = 0;
    size_t read_size;
    while(1) {
        if(feof(handle)) {
            break;
        }
        if(ferror(handle)) {
            PERROR("Error while reading file");
            abort();
        }
        read_size = min2(size - ptr, READ_BLOCK_SIZE);
        if(read_size <= 0)
            break;
        ptr += fread(buffer + ptr, 1, read_size, handle);
    }
}

void file_close(FILE *handle) {
    fclose(handle);
}
