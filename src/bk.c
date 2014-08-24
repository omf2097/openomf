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

int sd_bk_create(sd_bk_file *bk) {
    if(bk == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear everything
    memset(bk, 0, sizeof(sd_bk_file));
    return SD_SUCCESS;
}

int sd_bk_copy(sd_bk_file *dst, const sd_bk_file *src) {
    int ret;
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear everything
    memset(dst, 0, sizeof(sd_bk_file));

    // Int stuff
    dst->file_id = src->file_id;
    dst->unknown_a = src->unknown_a;
    dst->palette_count = src->palette_count;

    // Copy sounds
    memcpy(dst->soundtable, src->soundtable, sizeof(src->soundtable));

    // Copy animations
    for(int i = 0; i < MAX_BK_ANIMS; i++) {
        if(src->anims[i] != NULL) {
            if((dst->anims[i] = malloc(sizeof(sd_bk_anim))) == NULL) {
                return SD_OUT_OF_MEMORY;
            }
            if((ret = sd_bk_anim_copy(dst->anims[i], src->anims[i])) != SD_SUCCESS) {
                return ret;
            }
        }
    }

    // Copy background
    if(src->background != NULL) {
        if((dst->background = malloc(sizeof(sd_vga_image))) == NULL) {
            return SD_OUT_OF_MEMORY;
        }
        if((ret = sd_vga_image_copy(dst->background, src->background)) != SD_SUCCESS) {
            return ret;
        }
    }

    // Copy palettes
    for(int i = 0; i < MAX_BK_PALETTES; i++) {
        dst->palettes[i] = NULL;
        if(src->palettes[i] != NULL) {
            if((dst->palettes[i] = malloc(sizeof(sd_palette))) == NULL) {
                return SD_OUT_OF_MEMORY;
            }
            memcpy(dst->palettes[i], src->palettes[i], sizeof(sd_palette));
        }
    }

    return SD_SUCCESS;
}

void sd_bk_postprocess(sd_bk_file *bk) {
    char *table[1000] = {0}; // temporary lookup table
    sd_animation *anim;
    // fix NULL pointers for any 'missing' sprites
    for(int i = 0; i < MAX_BK_ANIMS; i++) {
        if(bk->anims[i] != NULL) {
            anim = bk->anims[i]->animation;
            for(int j = 0; j < anim->sprite_count; j++) {
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
    uint16_t img_w, img_h;
    uint8_t animno = 0;
    sd_reader *r;
    int ret;

    // Initialize reader
    if(!(r = sd_reader_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    img_w = sd_read_uword(r);
    img_h = sd_read_uword(r);

    // Read animations
    while(1) {
        sd_skip(r, 4); // offset of next animation
        animno = sd_read_ubyte(r);
        if(animno >= MAX_BK_ANIMS || !sd_reader_ok(r)) {
            break;
        }

        // Initialize animation
        if((bk->anims[animno] = malloc(sizeof(sd_bk_anim))) == NULL) {
            return SD_OUT_OF_MEMORY;
        }
        if((ret = sd_bk_anim_create(bk->anims[animno])) != SD_SUCCESS) {
            return ret;
        }
        if((ret = sd_bk_anim_load(r, bk->anims[animno])) != SD_SUCCESS) {
            return ret;
        }
    }

    // Read background image
    if((bk->background = malloc(sizeof(sd_vga_image))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    if((ret = sd_vga_image_create(bk->background, img_w, img_h)) != SD_SUCCESS) {
        return ret;
    }
    sd_read_buf(r, bk->background->data, img_h * img_w);

    // Read palettes
    bk->palette_count = sd_read_ubyte(r);
    for(uint8_t i = 0; i < bk->palette_count; i++) {
        if((bk->palettes[i] = malloc(sizeof(sd_palette))) == NULL) {
            return SD_OUT_OF_MEMORY;
        }
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

int sd_bk_save(const sd_bk_file *bk, const char* filename) {
    long rpos = 0;
    long opos = 0;
    sd_writer *w;
    int ret;

    if(!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write header
    sd_write_udword(w, bk->file_id);
    sd_write_ubyte(w, bk->unknown_a);

    // Write background size. In practice, this is always 320x200.
    // Still, let's take it srsly.
    if(bk->background != NULL) {
        sd_write_uword(w, bk->background->w);
        sd_write_uword(w, bk->background->h);
    } else {
        sd_write_uword(w, 320);
        sd_write_uword(w, 200);
    }

    // Write animations
    for(uint8_t i = 0; i < MAX_BK_ANIMS; i++) {
        if(bk->anims[i] != NULL) {
            opos = sd_writer_pos(w); // remember where we need to fill in the blank
            sd_write_udword(w, 0); // write a 0 as a placeholder
            sd_write_ubyte(w, i);
            if((ret = sd_bk_anim_save(w, bk->anims[i])) != SD_SUCCESS) {
                return ret;
            }
            rpos = sd_writer_pos(w);
            sd_writer_seek_start(w, opos);
            sd_write_udword(w, rpos); // write the actual size
            sd_writer_seek_start(w, rpos);
        }
    }
    sd_write_udword(w, rpos);
    sd_write_ubyte(w, MAX_BK_ANIMS+1); // indicate end of animations

    // Write background image. If none exists, write black image.
    if(bk->background != NULL) {
        sd_write_buf(w, bk->background->data, bk->background->len);
    } else {
        sd_write_fill(w, 0, 64000);
    }

    // Write palettes
    sd_write_ubyte(w, bk->palette_count);
    for(uint8_t i = 0; i < bk->palette_count; i++) {
        sd_palette_save(w, bk->palettes[i]);
    }

    // Write soundtable
    sd_write_buf(w, bk->soundtable, 30);

    // All done, close writer
    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_bk_set_background(sd_bk_file *bk, const sd_vga_image *img) {
    int ret;
    if(bk == NULL) {
        return SD_INVALID_INPUT;
    }
    if(bk->background != NULL) {
        sd_vga_image_free(bk->background);
        free(bk->background);
    }
    if(img == NULL) {
        return SD_SUCCESS;
    }
    if((bk->background = malloc(sizeof(sd_vga_image))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    if((ret = sd_vga_image_copy(bk->background, img)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_vga_image* sd_bk_get_background(const sd_bk_file *bk) {
    return bk->background;
}

int sd_bk_set_anim(sd_bk_file *bk, int index, const sd_bk_anim *anim) {
    int ret;
    if(index < 0 || index >= MAX_BK_ANIMS || bk == NULL) {
        return SD_INVALID_INPUT;
    }
    if(bk->anims[index] != NULL) {
        sd_bk_anim_free(bk->anims[index]);
        free(bk->anims[index]);
        bk->anims[index] = NULL;
    }
    // If input was NULL, we want to stop here.
    if(anim == NULL) {
        return SD_SUCCESS;
    }
    if((bk->anims[index] = malloc(sizeof(sd_bk_anim))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    if((ret = sd_bk_anim_copy(bk->anims[index], anim)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_bk_anim* sd_bk_get_anim(const sd_bk_file *bk, int index) {
    if(index < 0 || index >= MAX_BK_ANIMS || bk == NULL) {
        return NULL;
    }
    return bk->anims[index];
}

int sd_bk_set_palette(sd_bk_file *bk, int index, const sd_palette *palette) {
    if(index < 0 || index >= bk->palette_count || bk == NULL || palette == NULL) {
        return SD_INVALID_INPUT;
    }
    if(bk->palettes[index] != NULL) {
        free(bk->palettes[index]);
    }
    if((bk->palettes[index] = malloc(sizeof(sd_palette))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memcpy(bk->palettes[index], palette, sizeof(sd_palette));
    return SD_SUCCESS;
}

int sd_bk_pop_palette(sd_bk_file *bk) {
    if(bk == NULL || bk->palette_count <= 0) {
        return SD_INVALID_INPUT;
    }

    bk->palette_count--;
    free(bk->palettes[bk->palette_count]);
    bk->palettes[bk->palette_count] = NULL;

    return SD_SUCCESS;
}

int sd_bk_push_palette(sd_bk_file *bk, const sd_palette *palette) {
    if(bk == NULL || palette == NULL || bk->palette_count >= MAX_BK_PALETTES) {
        return SD_INVALID_INPUT;
    }
    if(bk->palettes[bk->palette_count] != NULL) {
        free(bk->palettes[bk->palette_count]);
    }
    if((bk->palettes[bk->palette_count] = malloc(sizeof(sd_palette))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memcpy(bk->palettes[bk->palette_count], palette, sizeof(sd_palette));
    bk->palette_count++;

    return SD_SUCCESS;
}

sd_palette* sd_bk_get_palette(const sd_bk_file *bk, int index) {
    if(bk == NULL || index < 0 || index >= bk->palette_count) {
        return NULL;
    }
    return bk->palettes[index];
}

void sd_bk_free(sd_bk_file *bk) {
    int i;
    if(bk->background != NULL) {
        sd_vga_image_free(bk->background);
    }
    for(i = 0; i < MAX_BK_ANIMS; i++) {
        if(bk->anims[i] != NULL) {
            sd_bk_anim_free(bk->anims[i]);
            free(bk->anims[i]);
        }
    }
    for(i = 0; i < bk->palette_count; i++) {
        if(bk->palettes[i] != NULL) {
            free(bk->palettes[i]);
        }
    }
}
