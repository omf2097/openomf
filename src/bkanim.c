#include "bkanim.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "error.h"
#include <stdlib.h>

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
    bka->unknown_a = sd_read_ubyte(r);
    bka->unknown_b = sd_read_ubyte(r);
    bka->unknown_c = sd_read_ubyte(r);
    bka->unknown_d = sd_read_uword(r);
    bka->unknown_e = sd_read_ubyte(r);
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
    sd_write_ubyte(writer, bka->unknown_a);
    sd_write_ubyte(writer, bka->unknown_b);
    sd_write_ubyte(writer, bka->unknown_c);
    sd_write_uword(writer, bka->unknown_d);
    sd_write_ubyte(writer, bka->unknown_e);
    sd_write_uword(writer, bka->unknown_size);
    sd_write_buf(writer, bka->unknown_data, bka->unknown_size);

    // Write animation
    sd_animation_save(writer, bka->animation);
}
