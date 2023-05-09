#include "formats/bk.h"
#include "resources/bk.h"

void bk_create(bk *b, void *src) {
    sd_bk_file *sdbk = (sd_bk_file *)src;

    // File ID
    b->file_id = sdbk->file_id;

    // Copy VGA image
    surface_create_from_data(&b->background, SURFACE_TYPE_PALETTE, sdbk->background->w, sdbk->background->h,
                             sdbk->background->data);

    // Copy sound translation table
    memcpy(b->sound_translation_table, sdbk->soundtable, 30);

    // Copy palettes
    vector_create(&b->palettes, sizeof(palette));
    for(int i = 0; i < sdbk->palette_count; i++) {
        vector_append(&b->palettes, (palette *)sdbk->palettes[i]);
    }

    // All scenes always have the menu colors set for palette 0.
    palette *pal = vector_get(&b->palettes, 0);
    palette_set_menu_colors(pal);

    // Index 0 is always black.
    pal->data[0][0] = pal->data[0][1] = pal->data[0][2] = 0;

    // Copy info structs
    hashmap_create(&b->infos);
    bk_info tmp_bk_info;
    for(int i = 0; i < 50; i++) {
        if(sdbk->anims[i] != NULL) {
            bk_info_create(&tmp_bk_info, (void *)sdbk->anims[i], i);
            hashmap_iput(&b->infos, i, &tmp_bk_info, sizeof(bk_info));
        }
    }
}

bk_info *bk_get_info(bk *b, int id) {
    bk_info *val;
    unsigned int tmp;
    if(hashmap_iget(&b->infos, id, (void **)&val, &tmp) == 1) {
        return NULL;
    }
    return val;
}

palette *bk_get_palette(bk *b, int id) {
    return vector_get(&b->palettes, id);
}

char *bk_get_stl(bk *b) {
    return b->sound_translation_table;
}

void bk_free(bk *b) {
    surface_free(&b->background);
    vector_free(&b->palettes);

    // Free info structs
    iterator it;
    hashmap_iter_begin(&b->infos, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        bk_info_free((bk_info *)pair->value);
    }
    hashmap_free(&b->infos);
}
