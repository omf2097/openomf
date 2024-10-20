#ifndef SGMANAGER_H
#define SGMANAGER_H

#include "formats/chr.h"
#include "utils/list.h"

int sg_init(void);
int sg_count(void);
list *sg_load_all(void);
int sg_load(sd_chr_file *chr, const char *pilotname);
int sg_save(sd_chr_file *chr);
int sg_delete(const char *pilotname);

#endif // SGMANAGER_H
