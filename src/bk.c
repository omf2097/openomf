#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/palette.h"
#include "shadowdive/rgba_image.h"
#include "shadowdive/vga_image.h"
#include "shadowdive/animation.h"
#include "shadowdive/bkanim.h"
#include "shadowdive/error.h"
#include "shadowdive/bk.h"

sd_bk_file* sd_bk_create() {
    sd_bk_file *bk = (sd_bk_file*)malloc(sizeof(sd_bk_file));
    memset(bk->anims, 0, sizeof(bk->anims));
    memset(bk->palettes, 0, sizeof(bk->palettes));
    memset(bk->soundtable, 0, sizeof(bk->soundtable));
    bk->background = NULL;
    bk->file_id = 0;
    bk->unknown_a = 0;
    bk->num_palettes = 0;
    return bk;
}

void sd_bk_postprocess(sd_bk_file *bk) {
    char *table[1000] = {0}; // temporary lookup table
    sd_animation *anim;
    // fix NULL pointers for any 'missing' sprites
    for(int i = 0; i < MAX_BK_ANIMS; i++) {
        if(bk->anims[i] != NULL) {
            anim = bk->anims[i]->animation;
            for(int j = 0; j < anim->frame_count; j++) {
                if(anim->sprites[j]->missing > 0) {
                    if(table[anim->sprites[j]->index]) {
                        anim->sprites[j]->data = table[anim->sprites[j]->index];
                    }
                } else {
                    table[anim->sprites[j]->index] = anim->sprites[j]->data;
                }
            }
        }
    }
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
        if(animno >= MAX_BK_ANIMS || !sd_reader_ok(r)) {
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
    for(uint8_t i = 0; i < bk->num_palettes; i++) {
        bk->palettes[i] = (sd_palette*)malloc(sizeof(sd_palette));
        sd_palette_load(r, bk->palettes[i]);
    }

    // Read soundtable
    sd_read_buf(r, bk->soundtable, 30);

    // Fix missing sprites
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
    for(uint8_t i = 0; i < MAX_BK_ANIMS; i++) {
        if (bk->anims[i]) { // there can be gaps
            opos = sd_writer_pos(w); // remember where we need to fill in the blank
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
    sd_write_ubyte(w, MAX_BK_ANIMS+1); // indicate end of animations

    // Write background image
    sd_write_buf(w, bk->background->data, bk->background->len);

    // Write palettes
    sd_write_ubyte(w, bk->num_palettes);
    for(uint8_t i = 0; i < bk->num_palettes; i++) {
        sd_palette_save(w, bk->palettes[i]);
    }

    // Write soundtable
    sd_write_buf(w, bk->soundtable, 30);

    // All done, close writer
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_bk_set_background(sd_bk_file *bk, sd_vga_image *img) {
    if(bk->background != NULL) {
        sd_vga_image_delete(bk->background);
    }
    bk->background = img;
}

sd_vga_image* sd_bk_get_background(sd_bk_file *bk) {
    return bk->background;
}

void sd_bk_set_anim(sd_bk_file *bk, int index, sd_bk_anim *anim) {
    if(index < 0 || index >= MAX_BK_ANIMS || bk == NULL)
        return;

    if(bk->anims[index] != NULL) {
        sd_bk_anim_delete(bk->anims[index]);
    }
    bk->anims[index] = anim;
}

sd_bk_anim* sd_bk_get_anim(sd_bk_file *bk, int index) {
    if(index < 0 || index >= MAX_BK_ANIMS || bk == NULL)
        return NULL;
    return bk->anims[index];
}

void sd_bk_set_palette(sd_bk_file *bk, int index, sd_palette *palette) {
    if(index < 0 || index >= MAX_BK_PALETTES || bk == NULL || palette == NULL)
        return;

    if(bk->palettes[index] != NULL) {
        free(bk->palettes[index]);
    }
    bk->palettes[index] = palette;
}

void sd_bk_pop_palette(sd_bk_file *bk) {
    if(bk == NULL || bk->num_palettes <= 0)
        return;
    bk->num_palettes--;
    free(bk->palettes[bk->num_palettes]);
    bk->palettes[bk->num_palettes] = NULL;
}

void sd_bk_push_palette(sd_bk_file *bk, sd_palette *palette) {
    if(bk == NULL || palette == NULL || bk->num_palettes >= MAX_BK_PALETTES)
        return;
    bk->palettes[bk->num_palettes++] = palette;
}

sd_palette* sd_bk_get_palette(sd_bk_file *bk, int index) {
    if(bk == NULL || index < 0 || index >= bk->num_palettes)
        return NULL;
    return bk->palettes[index];
}

void sd_bk_delete(sd_bk_file *bk) {
    if(bk->background) {
        sd_vga_image_delete(bk->background);
    }
    for(int i = 0; i < MAX_BK_ANIMS; i++) {
        if(bk->anims[i] != NULL) {
            sd_bk_anim_delete(bk->anims[i]);
        }
    }
    for(int i = 0; i < bk->num_palettes; i++) {
        free(bk->palettes[i]);
    }
    free(bk);
}
