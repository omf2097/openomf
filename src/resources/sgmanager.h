#ifndef SGMANAGER_H
#define SGMANAGER_H

#include "formats/chr.h"
#include "utils/list.h"

int sg_init();
int sg_count();
list *sg_load_all();
int sg_load(sd_chr_file *chr, const char *pilotname);
int sg_delete(const char *pilotname);

#endif // SGMANAGER_H
