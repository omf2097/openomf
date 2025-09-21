#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "formats/chr.h"
#include "resource_files.h"
#include "utils/hashmap.h"

bool modmanager_init(void);

bool modmanager_get_bk_background(int file_id, sd_vga_image **img);

#endif // MODMANAGER_H
