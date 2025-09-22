#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "formats/chr.h"
#include "resource_files.h"
#include "resources/animation.h"
#include "utils/hashmap.h"

bool modmanager_init(void);

bool modmanager_get_bk_background(int file_id, sd_vga_image **img);
bool modmanager_get_sprite(animation_source source, int file_id, int animation, int frame, sd_sprite **spr);
unsigned int modmanager_count_music(str *name);
bool modmanager_get_music(str *name, unsigned int index, unsigned char **buf, size_t *buflen);

#endif // MODMANAGER_H
