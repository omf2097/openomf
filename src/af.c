#include "af.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include "error.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

sd_af_file* sd_af_create() {
    sd_af_file *af = (sd_af_file*)malloc(sizeof(sd_af_file));
    memset(af->moves, 0, sizeof(af->moves));
    return af;
}

int sd_af_load(sd_af_file *af, const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    af->file_id = sd_read_uword(r);
    af->unknown_a = sd_read_uword(r);
    af->endurance = sd_read_udword(r);
    af->unknown_b = sd_read_ubyte(r); // TODO: Find out what this is
    af->power = sd_read_uword(r);
    af->forward_speed = sd_read_dword(r);
    af->reverse_speed = sd_read_dword(r);
    af->jump_speed = sd_read_dword(r);
    af->fall_speed = sd_read_dword(r);
    af->unknown_c = sd_read_uword(r); // TODO: Find out what this is

    // Read animations
    uint8_t moveno = 0;
    while(1) {
        moveno = sd_read_ubyte(r);
        if(moveno >= 70 || !sd_reader_ok(r)) {
            break;
        }

        // Read move
        af->moves[moveno] = sd_move_create();
        int ret = sd_move_load(r, af->moves[moveno]);
        if(ret != 0) {
            return ret;
        }
    }

    // Read footer
    sd_read_buf(r, af->footer, 30);

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_af_save(sd_af_file *af, const char* filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    sd_write_uword(w, af->file_id);
    sd_write_uword(w, af->unknown_a);
    sd_write_uword(w, af->endurance);
    sd_write_ubyte(w, af->unknown_b);
    sd_write_uword(w, af->power);
    sd_write_dword(w, af->forward_speed);
    sd_write_dword(w, af->reverse_speed);
    sd_write_dword(w, af->jump_speed);
    sd_write_dword(w, af->fall_speed);
    sd_write_uword(w, af->unknown_c);

    // Write animations
    for(uint8_t i = 0; i < 70; i++) {
        if(af->moves[i]) {
            sd_write_ubyte(w, i);
            sd_move_save(w, af->moves[i]);
        }
    }

    // Footer
    sd_write_buf(w, af->footer, 30);

    // All done!
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_af_delete(sd_af_file *af) {
    for(int i = 0; i < 70; i++) {
        if(af->moves[i]) {
            sd_move_delete(af->moves[i]);
        }
    }
    free(af);
}
