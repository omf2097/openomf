#ifndef SGMANAGER_H
#define SGMANAGER_H

#include "formats/chr.h"
#include "utils/list.h"

int sg_count(void);
list *sg_load_all(void);
int sg_load(sd_chr_file *chr, const path *file_name);
int sg_load_pilot(sd_chr_file *chr, const char *pilot_name);
int sg_save(sd_chr_file *chr);
int sg_delete(const char *pilot_name);

#endif // SGMANAGER_H
