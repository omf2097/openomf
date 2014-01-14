#include <string.h>
#include <stdlib.h>
#include <shadowdive/shadowdive.h>
#include "resources/bk.h"

void bk_create(bk *b, void *src) {
    sd_bk_file *sdbk = (sd_bk_file*)src;

    // File ID
    b->file_id = sdbk->file_id;

    // Copy VGA image
    surface *bk_surface = malloc(sizeof(surface));
    surface_create_from_data(
        bk_surface, 
        SURFACE_TYPE_PALETTE, 
        sdbk->background->w, 
        sdbk->background->h, 
        sdbk->background->data);
    sprite_create_custom(&b->background, vec2i_create(0,0), bk_surface);

    // Copy sound translation table
    memcpy(b->sound_translation_table, sdbk->soundtable, 30);

    // Copy palettes
    vector_create(&b->palettes, sizeof(palette));
    for(int i = 0; i < sdbk->num_palettes; i++) {
        vector_append(&b->palettes, (palette*)sdbk->palettes[i]);
    }

    // Copy info structs
    hashmap_create(&b->infos, 7);
    bk_info tmp_bk_info;
    for(int i = 0; i < 50; i++) {
        if(sdbk->anims[i] != NULL) {
            bk_info_create(&tmp_bk_info, (void*)sdbk->anims[i], i);
            hashmap_iput(&b->infos, i, &tmp_bk_info, sizeof(bk_info));
        }
    }
}

bk_info* bk_get_info(bk *b, int id) {
    bk_info *val;
    unsigned int tmp;
    if(hashmap_iget(&b->infos, id, (void**)&val, &tmp) == 1) {
        return NULL;
    }
    return val;
}

palette* bk_get_palette(bk *b, int id) {
    return vector_get(&b->palettes, id);
}

char* bk_get_stl(bk *b) {
    return b->sound_translation_table;
}

void bk_free(bk *b) {
    sprite_free(&b->background);
    vector_free(&b->palettes);

    // Free info structs
    iterator it;
    hashmap_iter_begin(&b->infos, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        bk_info_free((bk_info*)pair->val);
    }
    hashmap_free(&b->infos);
}
