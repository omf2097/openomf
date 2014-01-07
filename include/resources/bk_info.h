#ifndef _BK_INFO_H
#define _BK_INFO_H

#include "resources/animation.h"

typedef struct bk_info_t {
    unsigned int chain_hit;
    unsigned int chain_no_hit;
    unsigned int load_on_start;
    unsigned int probability;
    unsigned int hazard_damage;
    str footer_string;
    animation ani;
} bk_info;

void bk_info_create(bk_info *info, void *src, int id);
void bk_info_free(bk_info *info);

#endif // _BK_INFO_H
