#ifndef MODMANAGER_H
#define MODMANAGER_H

#include "formats/chr.h"
#include "formats/pic.h"
#include "resource_files.h"
#include "resources/af.h"
#include "resources/af_move.h"
#include "resources/animation.h"
#include "resources/bk_info.h"
#include "utils/hashmap.h"

bool modmanager_init(void);

bool modmanager_get_bk_background(str *name, sd_vga_image **img);
bool modmanager_get_sprite(animation_source source, str *name, int animation, int frame, sd_sprite **spr);
unsigned int modmanager_count_music(str *name);
bool modmanager_get_music(str *name, unsigned int index, unsigned char **buf, size_t *buflen);

bool modmanager_get_af_move(str *name, int move_id, af_move *move_data);
bool modmanager_get_bk_animation(str *name, int anim_id, bk_info *bk_data);

bool modmanager_get_fighter_header(str *name, af *fighter);

bool modmanager_get_tournament_mod(const char *tournament_name, sd_tournament_file *tourn_data);

bool modmanager_get_player_pics(sd_pic_file *pic);

#endif // MODMANAGER_H
