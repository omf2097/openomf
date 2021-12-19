#ifndef SCANDIR_H
#define SCANDIR_H

#include "utils/list.h"

int scan_directory(list *dlist, const char *dir);
int scan_directory_prefix(list *dlist, const char *dir, const char *prefix);

#endif // SCANDIR_H
