#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "utils/array.h"

typedef struct sd_bk_anim_t sd_bk_anim;
typedef struct sd_stringparser_t sd_stringparser;
typedef struct sd_palette_t sd_palette;

typedef struct animation_t {
    sd_bk_anim *bka;
    sd_stringparser *parser;
    array sprites;
} animation;

void animation_create(animation *ani, sd_bk_anim *bka, sd_palette *pal, int overlay);
void animation_free(animation *ani);

#endif // _ANIMATION_H