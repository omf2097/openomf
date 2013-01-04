#include "shadowdive/move.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/helpers.h"
#include "shadowdive/error.h"
#include "shadowdive/animation.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

sd_move* sd_move_create() {
    sd_move *move = (sd_move*)malloc(sizeof(sd_move));
    move->footer_string = NULL;
    move->animation = NULL;
    return move;
}

void sd_move_delete(sd_move *move) {
    if(move->footer_string)
        free(move->footer_string);
    if(move->animation)
        sd_animation_delete(move->animation);
    free(move);
}

int sd_move_load(sd_reader *r, sd_move *move) {
    // Read animation
    move->animation = sd_animation_create();
    sd_animation_load(r, move->animation);

    // Move footer
    sd_read_buf(r, move->unknown, 21);
    sd_read_buf(r, move->move_string, 21);
    int len = sd_read_uword(r);
    if (len > 0) {
        move->footer_string = (char*)malloc(len);
        sd_read_buf(r, move->footer_string, len);
        // ensure it has a terminating NULL
        assert(move->footer_string[len-1] == '\0');
    } else {
        // no footer string
        move->footer_string = NULL;
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

    // Save move footer
    sd_write_buf(writer, move->unknown, 21);
    sd_write_buf(writer, move->move_string, 21);
    if (move->footer_string) {
        uint16_t fs_size = strlen(move->footer_string);
        sd_write_uword(writer, fs_size+1);
        sd_write_buf(writer, move->footer_string, fs_size+1);
    } else {
        sd_write_uword(writer, 0); // no footer string, thus a length of 0
    }
}

void sd_move_set_animation(sd_move *move, sd_animation *animation) {
    if(move->animation) {
        free(move->animation);
    }
    move->animation = animation;
}

void sd_move_set_footer_string(sd_move *move, const char* str) {
    alloc_or_realloc((void**)&move->footer_string, strlen(str)+1);
    strcpy(move->footer_string, str);
}

