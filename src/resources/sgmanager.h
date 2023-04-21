#ifndef SGMANAGER_H
#define SGMANAGER_H

#include "formats/chr.h"

int sg_init();
int sg_load(sd_chr_file *chr, const char *pilotname);
int sd_save(const sd_pilot *pilot, const char *pilotname);

#endif // SGMANAGER_H
