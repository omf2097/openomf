#ifndef UTILS_IO_H
#define UTILS_IO_H

#include <stdio.h>

FILE *file_open(const char *file_name, const char *mode);
long file_size(FILE *handle);
void file_read(FILE *handle, char *buffer, long size);
void file_close(FILE *handle);

#endif // UTILS_IO_H
