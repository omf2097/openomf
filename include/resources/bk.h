#ifndef BK_H
#define BK_H

#include "resources/bk_info.h"
#include "utils/hashmap.h"
#include "utils/vector.h"

typedef struct bk_t {
    int file_id;
    surface background;
    hashmap infos;
    vector palettes;
    char sound_translation_table[30];
} bk;

void bk_create(bk *b, void *src);
bk_info* bk_get_info(bk *b, int id);
palette *bk_get_palette(bk *b, int id);
char* bk_get_stl(bk *b);
void bk_free(bk *b);

#endif // BK_H
