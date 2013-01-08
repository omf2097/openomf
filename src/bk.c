#include "shadowdive/bk.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/palette.h"
#include "shadowdive/vga_image.h"
#include "shadowdive/bkanim.h"
#include "shadowdive/animation.h"
#include "shadowdive/error.h"
#include <stdlib.h>
#include <string.h>

void sd_bk_postprocess(sd_bk_file *bk) {
    char *table[1000] = {0}; // temporary lookup table
    sd_animation *anim;
    // fix NULL pointers for any 'missing' sprites
    for(int i = 0; i < 50; i++) {
        if(bk->anims[i]) {
            anim = bk->anims[i]->animation;
            for(int j = 0; j < anim->frame_count; j++) {
                if (anim->sprites[j]->missing > 0) {
                    if (table[anim->sprites[j]->index]) {
                        anim->sprites[j]->img->data = table[anim->sprites[j]->index];
                    }
                } else {
                    table[anim->sprites[j]->index] = anim->sprites[j]->img->data;
                }
            }
        }
    }
}

sd_bk_file* sd_bk_create() {
    sd_bk_file *bk = (sd_bk_file*)malloc(sizeof(sd_bk_file));
    memset(bk->anims, 0, sizeof(bk->anims));
    bk->palettes = 0;
    return bk;
}

int sd_bk_load(sd_bk_file *bk, const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    uint16_t img_w = sd_read_uword(r);
    uint16_t img_h = sd_read_uword(r);

    // Read animations
    uint8_t animno = 0;
    while(1) {
        sd_skip(r, 4); // offset of next animation
        animno = sd_read_ubyte(r);
        if(animno >= 50 || !sd_reader_ok(r)) {
            break;
        }

        // Initialize animation
        bk->anims[animno] = sd_bk_anim_create();
        int ret = sd_bk_anim_load(r, bk->anims[animno]);
        if(ret != 0) {
            return ret;
        }
    }

    // Read background image
    bk->background = sd_vga_image_create(img_w, img_h);
    sd_read_buf(r, bk->background->data, img_h * img_w);

    // Read palettes
    bk->num_palettes = sd_read_ubyte(r);
    bk->palettes = malloc(bk->num_palettes * sizeof(sd_palette*));
    for(uint8_t i = 0; i < bk->num_palettes; i++) {
        bk->palettes[i] = (sd_palette*)malloc(sizeof(sd_palette));
        sd_palette_load(r, bk->palettes[i]);
    }

    // Read footer
    sd_read_buf(r, bk->footer, 30);

    sd_bk_postprocess(bk);

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_bk_save(sd_bk_file *bk, const char* filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write header
    sd_write_udword(w, bk->file_id);
    sd_write_ubyte(w, bk->unknown_a);
    sd_write_uword(w, bk->background->w);
    sd_write_uword(w, bk->background->h);

    // Write animations
    long rpos = 0, opos = 0;
    for(uint8_t i = 0; i < 50; i++) {
        if (bk->anims[i]) { // there can be gaps
            opos = sd_writer_pos(w); // remember whwre we need to fill in the blank
            sd_write_udword(w, 0); // write a 0 as a placeholder
            sd_write_ubyte(w, i);
            sd_bk_anim_save(w, bk->anims[i]);
            rpos = sd_writer_pos(w);
            sd_writer_seek_start(w, opos);
            sd_write_udword(w, rpos); // write the actual size
            sd_writer_seek_start(w, rpos);
        }
    }

    sd_write_udword(w, rpos);
    sd_write_ubyte(w, 51); // indicate end of animations

    // Write background image
    sd_write_buf(w, bk->background->data, bk->background->len);

    // Write palettes
    sd_write_ubyte(w, bk->num_palettes);
    for(uint8_t i = 0; i < bk->num_palettes; i++) {
        sd_palette_save(w, bk->palettes[i]);
    }

    // Write footer
    sd_write_buf(w, bk->footer, 30);

    // All done, close writer
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_bk_set_background(sd_bk_file *bk, sd_vga_image *img) {
    if(bk->background) {
        sd_vga_image_delete(bk->background);
    }
    bk->background = img;
}

void sd_bk_delete(sd_bk_file *bk) {
    if (bk->background) {
        sd_vga_image_delete(bk->background);
    }
    for(int i = 0; i < 50; i++) {
        if(bk->anims[i]) {
            sd_bk_anim_delete(bk->anims[i]);
        }
    }
    if(bk->palettes) {
        for(int i = 0; i < bk->num_palettes; i++) {
            free(bk->palettes[i]);
        }
        free(bk->palettes);
    }
    free(bk);
}
