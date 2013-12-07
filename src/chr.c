#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/chr.h"

#define UNUSED(x) (void)(x)

sd_chr_file* sd_chr_create() {
    sd_chr_file *chr = malloc(sizeof(sd_chr_file));
    memset(chr->name, 0, sizeof(chr->name));
    memset(chr->trn_name, 0, sizeof(chr->trn_name));
    memset(chr->trn_desc, 0, sizeof(chr->trn_desc));
    memset(chr->trn_image, 0, sizeof(chr->trn_image));
    chr->wins = 0;
    chr->losses = 0;
    chr->rank = 0;
    chr->har = 0;
    return chr;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure this looks like a CHR
    if(sd_read_udword(r) != 0xAFAEADAD) {
        return SD_FILE_PARSE_ERROR;
    }

    // Read player information
    sd_mreader *mr = sd_mreader_open_from_reader(r, 448);
    sd_mreader_xor(mr, 0xB0);

    // Get player stats
    sd_mread_buf(mr, chr->name, 16);
    sd_mskip(mr, 2);
    chr->wins = sd_mread_uword(mr);
    chr->losses = sd_mread_uword(mr);
    chr->rank = sd_mread_ubyte(mr);
    chr->har = sd_mread_ubyte(mr);

    uint16_t stats_a = sd_mread_uword(mr);
    uint16_t stats_b = sd_mread_uword(mr);
    uint16_t stats_c = sd_mread_uword(mr);
    uint8_t stats_d = sd_mread_ubyte(mr);
    chr->arm_power = (stats_a >> 0) & 0x1F;
    chr->leg_power = (stats_a >> 5) & 0x1F;
    chr->arm_speed = (stats_a >> 10) & 0x1F;
    chr->leg_speed = (stats_b >> 0) & 0x1F;
    chr->armor     = (stats_b >> 5) & 0x1F;
    chr->stun_resistance = (stats_b >> 10) & 0x1F;
    chr->power = (stats_c >> 0) & 0x7F;
    chr->agility = (stats_c >> 7) & 0x7F;
    chr->endurance = (stats_d >> 0) & 0x7F;

    sd_mskip(mr, 5);

    chr->credits = sd_mread_udword(mr);
    chr->color_1 = sd_mread_ubyte(mr);
    chr->color_2 = sd_mread_ubyte(mr);
    chr->color_3 = sd_mread_ubyte(mr);

    sd_mread_buf(mr, chr->trn_name, 13);
    sd_mread_buf(mr, chr->trn_desc, 31);
    sd_mread_buf(mr, chr->trn_image, 13);

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_delete(sd_chr_file *chr) {
    free(chr);
}
