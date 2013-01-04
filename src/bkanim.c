#include "shadowdive/bkanim.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/helpers.h"
#include "shadowdive/animation.h"
#include "shadowdive/error.h"
#include <stdlib.h>
#include <string.h>

sd_bk_anim* sd_bk_anim_create() {
    sd_bk_anim *bka = (sd_bk_anim*)malloc(sizeof(sd_bk_anim));
    bka->unknown_data = NULL;
    bka->animation = NULL;
    return bka;
}

void sd_bk_anim_delete(sd_bk_anim *bka) {
    if(bka->unknown_data)
        free(bka->unknown_data);
    if(bka->animation)
        sd_animation_delete(bka->animation);
    free(bka);
}

int sd_bk_anim_load(sd_reader *r, sd_bk_anim *bka) {
    // BK Specific animation header
    bka->null = sd_read_ubyte(r);
    bka->chain_hit = sd_read_ubyte(r);
    bka->chain_no_hit = sd_read_ubyte(r);
    bka->repeat = sd_read_ubyte(r);
    bka->probability = sd_read_uword(r);
    bka->hazard_damage = sd_read_ubyte(r);
    bka->unknown_size = sd_read_uword(r);
    bka->unknown_data = (char*)malloc(bka->unknown_size);
    sd_read_buf(r, bka->unknown_data, bka->unknown_size);

    // Initialize animation
    bka->animation = sd_animation_create();
    if(sd_animation_load(r, bka->animation)) {
        return SD_FILE_PARSE_ERROR;
    }

    // Return success
    return SD_SUCCESS;
}

void sd_bk_anim_save(sd_writer *writer, sd_bk_anim *bka) {
    // Write BK specific header
    sd_write_ubyte(writer, bka->null);
    sd_write_ubyte(writer, bka->chain_hit);
    sd_write_ubyte(writer, bka->chain_no_hit);
    sd_write_ubyte(writer, bka->repeat);
    sd_write_uword(writer, bka->probability);
    sd_write_ubyte(writer, bka->hazard_damage);
    sd_write_uword(writer, bka->unknown_size);
    sd_write_buf(writer, bka->unknown_data, bka->unknown_size);

    // Write animation
    sd_animation_save(writer, bka->animation);
}

void set_bk_anim_string(sd_bk_anim *bka, const char *data) {
    bka->unknown_size = strlen(data)+1;
    alloc_or_realloc((void**)&bka->unknown_data, bka->unknown_size);
    strcpy(bka->unknown_data, data);
}
