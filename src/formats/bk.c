#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "formats/animation.h"
#include "formats/bk.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/palette.h"
#include "formats/rgba_image.h"
#include "formats/vga_image.h"
#include "utils/allocator.h"

int sd_bk_create(sd_bk_file *bk) {
    if (bk == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear everything
    memset(bk, 0, sizeof(sd_bk_file));
    return SD_SUCCESS;
}

int sd_bk_copy(sd_bk_file *dst, const sd_bk_file *src) {
    int ret;
    if (dst == NULL || src == NULL) {
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
    for (int i = 0; i < MAX_BK_ANIMS; i++) {
        if (src->anims[i] != NULL) {
            dst->anims[i] = omf_calloc(1, sizeof(sd_bk_anim));
            if ((ret = sd_bk_anim_copy(dst->anims[i], src->anims[i])) != SD_SUCCESS) {
                return ret;
            }
        }
    }

    // Copy background
    if (src->background != NULL) {
        dst->background = omf_calloc(1, sizeof(sd_vga_image));
        if ((ret = sd_vga_image_copy(dst->background, src->background)) != SD_SUCCESS) {
            return ret;
        }
    }

    // Copy palettes
    for (int i = 0; i < MAX_BK_PALETTES; i++) {
        dst->palettes[i] = NULL;
        if (src->palettes[i] != NULL) {
            dst->palettes[i] = omf_calloc(1, sizeof(palette));
            memcpy(dst->palettes[i], src->palettes[i], sizeof(palette));
        }
    }

    return SD_SUCCESS;
}

void sd_bk_postprocess(sd_bk_file *bk) {
    char *table[1000] = {0}; // temporary lookup table
    sd_animation *anim;
    // fix NULL pointers for any 'missing' sprites
    for (int i = 0; i < MAX_BK_ANIMS; i++) {
        if (bk->anims[i] != NULL) {
            anim = bk->anims[i]->animation;
            for (int j = 0; j < anim->sprite_count; j++) {
                if (anim->sprites[j]->missing > 0) {
                    if (table[anim->sprites[j]->index]) {
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
    int ret = SD_SUCCESS;

    // Initialize reader
    if (!(r = sd_reader_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    img_w = sd_read_uword(r);
    img_h = sd_read_uword(r);

    // Read animations
    while (1) {
        sd_skip(r, 4); // offset of next animation
        animno = sd_read_ubyte(r);
        if (animno >= MAX_BK_ANIMS || !sd_reader_ok(r)) {
            break;
        }

        // Initialize animation
        bk->anims[animno] = omf_calloc(1, sizeof(sd_bk_anim));
        if ((ret = sd_bk_anim_create(bk->anims[animno])) != SD_SUCCESS) {
            goto exit_0;
        }
        if ((ret = sd_bk_anim_load(r, bk->anims[animno])) != SD_SUCCESS) {
            goto exit_0;
        }
    }

    // Read background image
    bk->background = omf_calloc(1, sizeof(sd_vga_image));
    if ((ret = sd_vga_image_create(bk->background, img_w, img_h)) != SD_SUCCESS) {
        goto exit_0;
    }
    int bsize = img_w * img_h;
    sd_read_buf(r, bk->background->data, bsize);

    // Read palettes
    bk->palette_count = sd_read_ubyte(r);
    for (uint8_t i = 0; i < bk->palette_count; i++) {
        bk->palettes[i] = omf_calloc(1, sizeof(palette));
        if ((ret = palette_load(r, bk->palettes[i])) != SD_SUCCESS) {
            goto exit_0;
        }
    }

    // Read soundtable
    sd_read_buf(r, bk->soundtable, 30);

    // Fix missing sprites
    sd_bk_postprocess(bk);

exit_0:
    sd_reader_close(r);
    return ret;
}

int sd_bk_save(const sd_bk_file *bk, const char *filename) {
    long rpos = 0;
    long opos = 0;
    sd_writer *w;
    int ret;

    if (!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write header
    sd_write_udword(w, bk->file_id);
    sd_write_ubyte(w, bk->unknown_a);

    // Write background size. In practice, this is always 320x200.
    // Still, let's take it srsly.
    if (bk->background != NULL) {
        sd_write_uword(w, bk->background->w);
        sd_write_uword(w, bk->background->h);
    } else {
        sd_write_uword(w, 320);
        sd_write_uword(w, 200);
    }

    // Write animations
    for (uint8_t i = 0; i < MAX_BK_ANIMS; i++) {
        if (bk->anims[i] != NULL) {
            opos = sd_writer_pos(w); // remember where we need to fill in the blank
            if (opos < 0) {
                goto error;
            }
            sd_write_udword(w, 0); // write a 0 as a placeholder
            sd_write_ubyte(w, i);
            if ((ret = sd_bk_anim_save(w, bk->anims[i])) != SD_SUCCESS) {
                sd_writer_close(w);
                return ret;
            }
            rpos = sd_writer_pos(w);
            if (rpos < 0) {
                goto error;
            }
            if (sd_writer_seek_start(w, opos) < 0) {
                goto error;
            }
            sd_write_udword(w, rpos); // write the actual size
            if (sd_writer_seek_start(w, rpos) < 0) {
                goto error;
            }
        }
    }
    sd_write_udword(w, rpos);
    sd_write_ubyte(w, MAX_BK_ANIMS + 1); // indicate end of animations

    // Write background image. If none exists, write black image.
    if (bk->background != NULL) {
        sd_write_buf(w, bk->background->data, bk->background->len);
    } else {
        sd_write_fill(w, 0, 64000);
    }

    // Write palettes
    sd_write_ubyte(w, bk->palette_count);
    for (uint8_t i = 0; i < bk->palette_count; i++) {
        palette_save(w, bk->palettes[i]);
    }

    // Write soundtable
    sd_write_buf(w, bk->soundtable, 30);

    if (sd_writer_errno(w)) {
        goto error;
    }

    // All done, close writer
    sd_writer_close(w);
    return SD_SUCCESS;

error:
    unlink(filename);
    sd_writer_close(w);
    return SD_FILE_WRITE_ERROR;
}

int sd_bk_set_background(sd_bk_file *bk, const sd_vga_image *img) {
    int ret;
    if (bk == NULL) {
        return SD_INVALID_INPUT;
    }
    if (bk->background != NULL) {
        sd_vga_image_free(bk->background);
        omf_free(bk->background);
    }
    if (img == NULL) {
        return SD_SUCCESS;
    }
    bk->background = omf_calloc(1, sizeof(sd_vga_image));
    if ((ret = sd_vga_image_copy(bk->background, img)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_vga_image *sd_bk_get_background(const sd_bk_file *bk) { return bk->background; }

int sd_bk_set_anim(sd_bk_file *bk, int index, const sd_bk_anim *anim) {
    int ret;
    if (index < 0 || index >= MAX_BK_ANIMS || bk == NULL) {
        return SD_INVALID_INPUT;
    }
    if (bk->anims[index] != NULL) {
        sd_bk_anim_free(bk->anims[index]);
        omf_free(bk->anims[index]);
    }
    // If input was NULL, we want to stop here.
    if (anim == NULL) {
        return SD_SUCCESS;
    }
    bk->anims[index] = omf_calloc(1, sizeof(sd_bk_anim));
    if ((ret = sd_bk_anim_copy(bk->anims[index], anim)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_bk_anim *sd_bk_get_anim(const sd_bk_file *bk, int index) {
    if (index < 0 || index >= MAX_BK_ANIMS || bk == NULL) {
        return NULL;
    }
    return bk->anims[index];
}

int sd_bk_set_palette(sd_bk_file *bk, int index, const palette *pal) {
    if (index < 0 || bk == NULL || index >= bk->palette_count || pal == NULL) {
        return SD_INVALID_INPUT;
    }
    if (bk->palettes[index] != NULL) {
        omf_free(bk->palettes[index]);
    }
    bk->palettes[index] = omf_calloc(1, sizeof(palette));
    memcpy(bk->palettes[index], pal, sizeof(palette));
    return SD_SUCCESS;
}

int sd_bk_pop_palette(sd_bk_file *bk) {
    if (bk == NULL || bk->palette_count <= 0) {
        return SD_INVALID_INPUT;
    }

    bk->palette_count--;
    omf_free(bk->palettes[bk->palette_count]);

    return SD_SUCCESS;
}

int sd_bk_push_palette(sd_bk_file *bk, const palette *pal) {
    if (bk == NULL || pal == NULL || bk->palette_count >= MAX_BK_PALETTES) {
        return SD_INVALID_INPUT;
    }
    if (bk->palettes[bk->palette_count] != NULL) {
        omf_free(bk->palettes[bk->palette_count]);
    }
    bk->palettes[bk->palette_count] = omf_calloc(1, sizeof(palette));
    memcpy(bk->palettes[bk->palette_count], pal, sizeof(palette));
    bk->palette_count++;

    return SD_SUCCESS;
}

palette *sd_bk_get_palette(const sd_bk_file *bk, int index) {
    if (bk == NULL || index < 0 || index >= bk->palette_count) {
        return NULL;
    }
    return bk->palettes[index];
}

void sd_bk_free(sd_bk_file *bk) {
    int i;
    if (bk->background != NULL) {
        sd_vga_image_free(bk->background);
        omf_free(bk->background);
    }
    for (i = 0; i < MAX_BK_ANIMS; i++) {
        if (bk->anims[i] != NULL) {
            sd_bk_anim_free(bk->anims[i]);
            omf_free(bk->anims[i]);
        }
    }
    for (i = 0; i < bk->palette_count; i++) {
        if (bk->palettes[i] != NULL) {
            omf_free(bk->palettes[i]);
        }
    }
}
