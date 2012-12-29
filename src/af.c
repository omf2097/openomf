#include "af.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

sd_af_file* sd_af_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    sd_af_file *af = malloc(sizeof(sd_af_file));

    // Header
    af->file_id = sd_read_uword(r);
    af->unknown_a = sd_read_uword(r);
    af->endurance = sd_read_udword(r);
    sd_skip(r, 1);// TODO: Find out what this is
    af->power = sd_read_uword(r);
    af->forward_speed = sd_read_dword(r);
    af->reverse_speed = sd_read_dword(r);
    af->jump_speed = sd_read_dword(r);
    af->fall_speed = sd_read_dword(r);
    sd_skip(r, 2);// TODO: Find out what this is
    memset(af->moves, 0, sizeof(af->moves));

    // Read animations
    uint8_t moveno = 0;
    while(1) {
        moveno = sd_read_ubyte(r);
        printf("move number %d\n", moveno);
        if(moveno >= 70 || !sd_reader_ok(r)) {
            break;
        }

        // Read move
        af->moves[moveno] = sd_move_create();
        sd_move_load(r, af->moves[moveno]);
    }

    // Read footer
    sd_read_buf(r, af->footer, 30);

    // Close & return
    sd_reader_close(r);
    return af;
}

int sd_af_save(const char* filename, sd_af_file *af) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return 0;
    }


    sd_writer_close(w);
    return 1;
}

void sd_af_delete(sd_af_file *af) {
    for(int i = 0; i < 70; i++) {
        if(af->moves[i]) {
            sd_move_delete(af->moves[i]);
        }
    }
    free(af);
}
