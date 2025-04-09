#ifndef UTILS_IO_H
#define UTILS_IO_H

#include <stdio.h>

FILE *file_open(const char *file_name, const char *mode);
long file_size(FILE *handle);
bool file_read(FILE *handle, char *buffer, long size);
void file_close(FILE *handle);

bool file_exists(const char *file_name);

#endif // UTILS_IO_H
