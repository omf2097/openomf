#ifndef _SCANDIR_H
#define _SCANDIR_H

#include "utils/list.h"

int scandir(list *dlist, const char *dir);
int scandir_prefix(list *dlist, const char *dir, const char *prefix);

#endif // _SCANDIR_H