#include "formats/bk.h"
#include "resources/bk.h"
#include "resources/modmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#ifdef USE_EXTENDED_PALETTE
#include "video/vga_extended_palette.h"
#endif
#include <string.h>

void bk_create(bk *b, void *src, str *name) {
    sd_bk_file *sdbk = (sd_bk_file *)src;

    // File ID
    b->file_id = sdbk->file_id;

    // Copy VGA image
    sd_vga_image *img;
    vga_palette *mod_pal = NULL;
    const vga_remap_table *mod_remap = NULL;
    int mod_sprite_type = 0;
    bool has_mod_background = modmanager_get_bk_background(name, &img, &mod_pal, &mod_remap, &mod_sprite_type);
    if(has_mod_background) {
        log_info("using modified BK background");
        surface_create_from_vga(&b->background, img);
        b->background.render_w = 320;
        b->background.render_h = 200;
    } else {
        surface_create_from_vga(&b->background, sdbk->background);
    }

    // Copy sound translation table
    memcpy(b->sound_translation_table, sdbk->soundtable, 30);

    // Copy palettes & remaps.
    vector_create_with_size(&b->palettes, sizeof(vga_palette), sdbk->palette_count);
    vector_create_with_size(&b->remaps, sizeof(vga_remap_tables), sdbk->palette_count);
    for(int i = 0; i < sdbk->palette_count; i++) {
        vector_append(&b->palettes, (vga_palette *)sdbk->palettes[i]);
        vector_append(&b->remaps, (vga_remap_tables *)sdbk->remaps[i]);
    }

#ifdef USE_EXTENDED_PALETTE
    // Store mod palette for later loading in scene_create,
    // after vga_state_set_base_palette_from has initialized the extended range.
    // Patch base palette with background colors (0x60-0x9F at identity).
    // Must happen AFTER palettes are copied from BK file above.
    if(has_mod_background && mod_pal) {
        b->mod_pal = mod_pal;
        b->mod_sprite_type = mod_sprite_type;

        vga_palette *bk_pal = (vga_palette *)vector_get(&b->palettes, 0);
        if(bk_pal) {
            for(int i = 0x60; i <= 0x9F; i++) {
                bk_pal->colors[i] = mod_pal->colors[i];
            }
            for(int i = 0xF4; i <= 0xF9; i++) {
                bk_pal->colors[i] = mod_pal->colors[i];
            }
        }
    }
#endif

    // Array for sprites, since we know we will fill most slots.
    array_create(&b->sprites);

    // Copy info structs
    hashmap_create(&b->infos);
    bk_info tmp_bk_info;
    for(int i = 0; i < 50; i++) {
        if(sdbk->anims[i] != NULL) {
            bk_info_create(name, &tmp_bk_info, &b->sprites, (void *)sdbk->anims[i], i);
            hashmap_put_int(&b->infos, i, &tmp_bk_info, sizeof(bk_info));
        }
    }
}

bk_info *bk_get_info(bk *b, int id) {
    bk_info *val;
    unsigned int tmp;
    if(hashmap_get_int(&b->infos, id, (void **)&val, &tmp) == 1) {
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
    foreach(it, pair) {
        bk_info_free((bk_info *)pair->value);
    }
    hashmap_free(&b->infos);

    array_free(&b->sprites);
}
