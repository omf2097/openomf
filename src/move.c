#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/helpers.h"
#include "shadowdive/error.h"
#include "shadowdive/animation.h"
#include "shadowdive/move.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int sd_move_create(sd_move *move) {
    move->unknown_0 = 0;
    move->unknown_2 = 0;
    move->unknown_3 = 0;
    move->unknown_4 = 0;
    move->unknown_5 = 0;
    move->unknown_6 = 0;
    move->unknown_7 = 0;
    move->unknown_8 = 0;
    move->unknown_9 = 0;
    move->unknown_10 = 0;
    move->unknown_11 = 0;
    move->next_anim_id = 0;
    move->category = 0;
    move->unknown_14 = 0;
    move->scrap_amount = 0;
    move->successor_id = 0;
    move->damage_amount = 0;
    move->unknown_18 = 0;
    move->unknown_19 = 0;
    move->points = 0;

    memset(move->move_string, 0, MOVE_STRING_MAX);
    memset(move->footer_string, 0, FOOTER_STRING_MAX);
    move->animation = NULL;
    return SD_SUCCESS;
}

void sd_move_free(sd_move *move) {
    if(move->animation) {
        sd_animation_free(move->animation);
    }
}

int sd_move_load(sd_reader *r, sd_move *move) {
    // Read animation
    move->animation = sd_animation_create();
    sd_animation_load(r, move->animation);

    // Header
    move->unknown_0 = sd_read_uword(r);
    move->unknown_2 = sd_read_uword(r);
    move->unknown_3 = sd_read_ubyte(r);
    move->unknown_4 = sd_read_ubyte(r);
    move->unknown_5 = sd_read_ubyte(r);
    move->unknown_6 = sd_read_ubyte(r);
    move->unknown_7 = sd_read_ubyte(r);
    move->unknown_8 = sd_read_ubyte(r);
    move->unknown_9 = sd_read_ubyte(r);
    move->unknown_10 = sd_read_ubyte(r);
    move->unknown_11 = sd_read_ubyte(r);
    move->next_anim_id = sd_read_ubyte(r);
    move->category = sd_read_ubyte(r);
    move->unknown_14 = sd_read_ubyte(r);
    move->scrap_amount = sd_read_ubyte(r);
    move->successor_id = sd_read_ubyte(r);
    move->damage_amount = sd_read_ubyte(r);
    move->unknown_18 = sd_read_ubyte(r);
    move->unknown_19 = sd_read_ubyte(r);
    move->points = sd_read_ubyte(r);

    // move string
    sd_read_buf(r, move->header, 21);
    sd_read_buf(r, move->move_string, 21);

    // Footer string
    int len = sd_read_uword(r);
    if(len > 0) {
        sd_read_buf(r, move->footer_string, len);
    }

    // Return success if reader is still ok
    if(!sd_reader_ok(r)) {
        return SD_FILE_PARSE_ERROR;
    }
    return SD_SUCCESS;
}

void sd_move_save(sd_writer *writer, sd_move *move) {
    // Save animation
    sd_animation_save(writer, move->animation);

    // Move header
    sd_write_uword(writer, move->unknown_0);
    sd_write_uword(writer, move->unknown_2);
    sd_write_ubyte(writer, move->unknown_3);
    sd_write_ubyte(writer, move->unknown_4);
    sd_write_ubyte(writer, move->unknown_5);
    sd_write_ubyte(writer, move->unknown_6);
    sd_write_ubyte(writer, move->unknown_7);
    sd_write_ubyte(writer, move->unknown_8);
    sd_write_ubyte(writer, move->unknown_9);
    sd_write_ubyte(writer, move->unknown_10);
    sd_write_ubyte(writer, move->unknown_11);
    sd_write_ubyte(writer, move->next_anim_id);
    sd_write_ubyte(writer, move->category);
    sd_write_ubyte(writer, move->unknown_14);
    sd_write_ubyte(writer, move->scrap_amount);
    sd_write_ubyte(writer, move->successor_id);
    sd_write_ubyte(writer, move->damage_amount);
    sd_write_ubyte(writer, move->unknown_18);
    sd_write_ubyte(writer, move->unknown_19);
    sd_write_ubyte(writer, move->points);

    // move string
    sd_write_buf(writer, move->move_string, 21);

    // Save footer string
    uint16_t fs_size = strlen(move->footer_string) + 1;
    sd_write_uword(writer, fs_size);
    sd_write_buf(writer, move->footer_string, fs_size);
}

int sd_move_set_animation(sd_move *move, const sd_animation *animation) {
    if(move->animation != NULL) {
        free(move->animation);
    }
    move->animation = malloc(sizeof(sd_animation));
    if(move->animetion == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memcpy(move->animation, animation, sizeof(sd_animation));
    return SD_SUCCESS;
}

int sd_move_set_footer_string(sd_move *move, const char* str) {
    if(strlen(str) >= FOOTER_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(move->footer_string, str);
    return SD_SUCCESS;
}

int sd_move_set_move_string(sd_move *move, const char *str) {
    if(strlen(str) >= MOVE_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(move->move_string, str);
    return SD_SUCCESS;
}
