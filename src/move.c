#include "move.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdlib.h>
#include <string.h>

sd_move* sd_move_create() {
    sd_move *move = (sd_move*)malloc(sizeof(sd_move));
    move->animation.overlay_table = NULL;
    move->footer_string = NULL;
    return move;
}

void sd_move_delete(sd_move *move) {
    if(move->animation.overlay_table)
        free(move->animation.overlay_table);
    if(move->footer_string)
        free(move->footer_string);
    free(move);
}

int sd_move_load(sd_reader *r, sd_move *move) {
    // Read animation
    sd_animation_load(r, &move->animation);

    // Move footer
    sd_read_buf(r, move->unknown, 21);
    sd_read_buf(r, move->move_string, 21);
    int len = sd_read_uword(r);
    move->footer_string = (char*)malloc(len+1);
    sd_read_buf(r, move->footer_string, len);
    move->footer_string[len] = '\0';

    // Return success if reader is still ok
    return sd_reader_ok(r);
}

void sd_move_save(sd_writer *writer, sd_move *move) {
    // Save animation
    sd_animation_save(writer, &move->animation);

    // Save move footer
    sd_write_buf(writer, move->unknown, 21);
    sd_write_buf(writer, move->move_string, 21);
    uint16_t fs_size = strlen(move->footer_string);
    sd_write_uword(writer, fs_size);
    sd_write_buf(writer, move->footer_string, fs_size);
    sd_write_ubyte(writer, 0);
}
