#ifndef SCANDIR_H
#define SCANDIR_H

#include "utils/list.h"

int scan_directory(list *dir_list, const char *dir);

/* Case insensitive scan for the file in path, modify the argument path. */
bool scan_directory_for_file(char *path, size_t path_size);

#endif // SCANDIR_H
