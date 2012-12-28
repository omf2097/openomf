#include "bk.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include <stdlib.h>
#include <assert.h>

sd_bk_file* sd_bk_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    sd_bk_file *bk = malloc(sizeof(sd_bk_file));

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    bk->img_w = sd_read_uword(r);
    bk->img_h = sd_read_uword(r);

    // Read animations
    uint8_t animno = 0;
    uint8_t size = 0;
    sd_animation *ani;
    while(1) {
        sd_skip(r, 4);
        animno = sd_read_ubyte(r);
        if(animno >= 50 || !sd_reader_ok(r)) {
            break;
        }
        // skip some unknown data, for now
        sd_skip(r, 7);
        size = sd_read_uword(r);
        // skip some unknown data
        sd_skip(r, size);
        ani = sd_animation_create();
        sd_read_buf(r, ani->unknown_a, 8);
        ani->overlay_count = sd_read_uword(r);
        ani->frame_count = sd_read_ubyte(r);
        ani->overlay_table = (uint32_t*)malloc(sizeof(uint32_t)*ani->overlay_count);
        sd_read_buf(r, (char*)ani->overlay_table, sizeof(uint32_t)*ani->overlay_count);
        ani->anim_string_len = sd_read_uword(r);
        ani->anim_string = (char*)malloc(ani->anim_string_len + 1);
        // assume its null terminates
        sd_read_buf(r, ani->anim_string, ani->anim_string_len+1);
        assert(ani->anim_string[ani->anim_string_len] == '\0');
        ani->extra_string_count = sd_read_ubyte(r);
        ani->extra_strings = (char**)malloc(sizeof(char*)*ani->extra_string_count);
        for(int i = 0; i < ani->extra_string_count; i++) {
            uint16_t size = sd_read_uword(r);
            ani->extra_strings[i] = malloc(size+1);
            // assume its null terminated
            sd_read_buf(r, ani->extra_strings[i], size+1);
            assert(ani->extra_strings[i][size] == '\0');
        }
        for(int i = 0; i < ani->frame_count; i++) {
            // finally, the actual sprite!
            sd_sprite *sprite = sd_sprite_create();
            uint16_t len = sd_read_uword(r);
            sprite->pos_x = sd_read_word(r);
            sprite->pos_y = sd_read_word(r);
            uint16_t width = sd_read_uword(r);
            uint16_t height = sd_read_uword(r);
            sprite->index = sd_read_ubyte(r);
            sprite->missing = sd_read_ubyte(r);
            if (sprite->missing == 0) {
                // sprite data follows
                sprite->img = sd_sprite_image_create(width, height, len);
                sd_read_buf(r, sprite->img->data, len);
                sd_sprite_image_decode(sprite->img, malloc(sizeof(sd_palette)), 1);
            } else {
                // TODO set the pointer to be the actual sprite, from the other animation, maybe?
                sprite->img = NULL;
            }
        }
    }

    // Read background image
    bk->background = sd_vga_image_create(bk->img_w, bk->img_h);
    sd_read_buf(r, bk->background->data, bk->img_h * bk->img_w);

    // Read palettes
    uint8_t num_palettes = sd_read_ubyte(r);
    bk->palettes = malloc(num_palettes * sizeof(sd_palette*));
    for(uint8_t i = 0; i < num_palettes; i++) {
        bk->palettes[i] = (sd_palette*)malloc(sizeof(sd_palette));
        sd_read_buf(r, (char*)bk->palettes[i]->data, 256*3);
        sd_read_buf(r, (char*)bk->palettes[i]->remaps, 19*256);
    }

    // Read footer
    sd_read_buf(r, bk->footer, 30);

    // Close & return
    sd_reader_close(r);
    return bk;
}

int sd_bk_save(const char* filename, sd_bk_file *bk) {
    return 0;
}

void sd_bk_delete(sd_bk_file *bk) {
    sd_vga_image_delete(bk->background);
    free(bk->palettes);
    free(bk);
}
