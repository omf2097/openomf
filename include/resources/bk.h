#ifndef _BK_H
#define _BK_H

#include "resources/bk_info.h"
#include "video/texture.h"
#include "utils/hashmap.h"
#include "utils/vector.h"

typedef struct bk_t {
	sprite background;
	hashmap infos;
	vector palettes;
	char sound_translation_table[30];
} bk;

void bk_create(bk *b, void *src);
bk_info* bk_get_info(bk *b, int id);
void bk_free(bk *b);

#endif // _BK_H