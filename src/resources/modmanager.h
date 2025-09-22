#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "formats/chr.h"
#include "resource_files.h"
#include "utils/hashmap.h"
#include "resources/animation.h"

bool modmanager_init(void);

bool modmanager_get_bk_background(int file_id, sd_vga_image **img);
bool modmanager_get_sprite(animation_source source, int file_id, int animation, int frame, sd_sprite **spr);
bool modmanager_get_music(str *name, unsigned char **buf, size_t *buflen);

#endif // MODMANAGER_H
