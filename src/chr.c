#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/memwriter.h"
#include "shadowdive/chr.h"

#define UNUSED(x) (void)(x)

int sd_chr_create(sd_chr_file *chr) {
    if(chr == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(chr, 0, sizeof(sd_chr_file));
    return SD_SUCCESS;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    if(chr == NULL ||filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Read up pilot block and the unknown data
    sd_mreader *mr = sd_mreader_open_from_reader(r, 448);
    sd_mreader_xor(mr, 0xAC);
    sd_pilot_create(&chr->pilot);
    sd_pilot_load_from_mem(mr, &chr->pilot);
    sd_mreader_close(mr);

    // Read enemies block
    mr = sd_mreader_open_from_reader(r, 68 * chr->pilot.enemies_inc_unranked);
    sd_mreader_xor(mr, (chr->pilot.enemies_inc_unranked * 68) & 0xFF);

    // Handle enemy data
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        // Reserve & zero out
        chr->enemies[i] = malloc(sizeof(sd_chr_enemy));
        sd_pilot_create(&chr->enemies[i]->pilot);
        sd_pilot_load_player_from_mem(mr, &chr->enemies[i]->pilot);
        sd_mread_buf(mr, chr->enemies[i]->unknown, 25);
    }

    // Close memory reader for enemy data block
    sd_mreader_close(mr);

    // Read HAR palette
    sd_palette_create(&chr->pal);
    sd_palette_load_range(r, &chr->pal, 0, 48);

    // No idea what this is.
    // TODO: Find out.
    chr->unknown_b = sd_read_udword(r);

    // Load sprite
    chr->photo = malloc(sizeof(sd_sprite));
    sd_sprite_create(chr->photo);
    int ret = sd_sprite_load(r, chr->photo);
    if(ret != SD_SUCCESS) {
        goto error_1;
    }

    // Fix photo size
    chr->photo->width++;
    chr->photo->height++;

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;


error_1:
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    if(chr == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Save pilot and unknown
    sd_mwriter *mw = sd_mwriter_open();
    sd_pilot_save_to_mem(mw, &chr->pilot);
    sd_mwriter_xor(mw, 0xAC);
    sd_mwriter_save(mw, w);
    sd_mwriter_close(mw);

    // Write enemy data
    mw = sd_mwriter_open();
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        sd_pilot_save_player_to_mem(mw, &chr->enemies[i]->pilot);
        sd_mwrite_buf(mw, chr->enemies[i]->unknown, 25);
    }
    sd_mwriter_xor(mw, (chr->pilot.enemies_inc_unranked * 68) & 0xFF);
    sd_mwriter_save(mw, w);
    sd_mwriter_close(mw);

    // Save palette
    sd_palette_save_range(w, &chr->pal, 0, 48);

    // Save this, whatever this is.
    sd_write_udword(w, chr->unknown_b);

    // Save photo. Hacky size fix.
    chr->photo->width--;
    chr->photo->height--;
    sd_sprite_save(w, chr->photo);
    chr->photo->width++;
    chr->photo->height++;

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_free(sd_chr_file *chr) {
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        if(chr->enemies[i] != NULL) {
            free(chr->enemies[i]);
        }
    }
    sd_sprite_free(chr->photo);
}

const sd_chr_enemy* sd_chr_get_enemy(sd_chr_file *chr, int enemy_num) {
    if(chr == NULL || enemy_num < 0 || enemy_num >= chr->pilot.enemies_inc_unranked) {
        return NULL;
    }
    return chr->enemies[enemy_num];
}