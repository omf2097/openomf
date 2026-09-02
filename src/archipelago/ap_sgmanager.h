#ifndef AP_SGMANAGER_H
#define AP_SGMANAGER_H

#include "formats/chr.h"
#include "utils/path.h"

path get_ap_save_directory(void);
int sg_save_as(sd_chr_file *chr, const char *filename_stem);
int sg_save_ap(sd_chr_file *chr, const char *filename_stem);
int sg_load_ap_pilot(sd_chr_file *chr, const char *pilot_name);

#endif // AP_SGMANAGER_H
