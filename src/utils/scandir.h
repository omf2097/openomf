#ifndef SCANDIR_H
#define SCANDIR_H

#include "utils/list.h"

int scan_directory(list *dir_list, const char *dir);
int scan_directory_prefix(list *dir_list, const char *dir, const char *prefix);
int scan_directory_suffix(list *dir_list, const char *dir, const char *suffix);

#endif // SCANDIR_H
