#include "formats/bk.h"
#include "resources/bk.h"
#include "utils/allocator.h"
#include <string.h>

void bk_create(bk *b, void *src) {
    sd_bk_file *sdbk = (sd_bk_file *)src;

    // File ID
    b->file_id = sdbk->file_id;

    // Copy VGA image
    surface_create_from_vga(&b->background, sdbk->background, sdbk->background->transparent);

    // Copy sound translation table
    memcpy(b->sound_translation_table, sdbk->soundtable, 30);

    // Copy palettes & remaps.
    vector_create_with_size(&b->palettes, sizeof(vga_palette), sdbk->palette_count);
    vector_create_with_size(&b->remaps, sizeof(vga_remap_tables), sdbk->palette_count);
    for(int i = 0; i < sdbk->palette_count; i++) {
        vector_append(&b->palettes, (vga_palette *)sdbk->palettes[i]);
        vector_append(&b->remaps, (vga_remap_tables *)sdbk->remaps[i]);
    }

    // Array for sprites, since we know we will fill most slots.
    array_create(&b->sprites);

    // Copy info structs
    hashmap_create(&b->infos);
    bk_info tmp_bk_info;
    for(int i = 0; i < 50; i++) {
        if(sdbk->anims[i] != NULL) {
            bk_info_create(&tmp_bk_info, &b->sprites, (void *)sdbk->anims[i], i);
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

vga_palette *bk_get_palette(bk *b, int id) {
    return vector_get(&b->palettes, id);
}

vga_remap_tables *bk_get_remaps(bk *b, int id) {
    return vector_get(&b->remaps, id);
}

char *bk_get_stl(bk *b) {
    return b->sound_translation_table;
}

void bk_free(bk *b) {
    surface_free(&b->background);
    vector_free(&b->palettes);
    vector_free(&b->remaps);

    // Free info structs
    iterator it;
    hashmap_iter_begin(&b->infos, &it);
    hashmap_pair *pair = NULL;
    while((pair = iter_next(&it)) != NULL) {
        bk_info_free((bk_info *)pair->value);
    }
    hashmap_free(&b->infos);

    array_free(&b->sprites);
}
