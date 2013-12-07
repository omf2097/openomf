#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/animation.h"
#include "shadowdive/error.h"
#include "shadowdive/move.h"
#include "shadowdive/af.h"

void sd_af_postprocess(sd_af_file *af) {
    char *table[1000] = {0}; // temporary lookup table
    sd_animation *anim;
    // fix NULL pointers for any 'missing' sprites
    for(int i = 0; i < 70; i++) {
        if(af->moves[i] != NULL) {
            anim = af->moves[i]->animation;
            for(int j = 0; j < anim->frame_count; j++) {
                if(anim->sprites[j]->missing > 0) {
                    if(table[anim->sprites[j]->index]) {
                        anim->sprites[j]->img->data = table[anim->sprites[j]->index];
                    }
                } else {
                    table[anim->sprites[j]->index] = anim->sprites[j]->img->data;
                }
            }
        }
    }
}

sd_af_file* sd_af_create() {
    sd_af_file *af = (sd_af_file*)malloc(sizeof(sd_af_file));
    memset(af->moves, 0, sizeof(af->moves));
    memset(af->soundtable, 0, sizeof(af->soundtable));
    af->file_id = 0;
    af->unknown_a = 0;
    af->endurance = 0;
    af->unknown_b = 0;
    af->power = 0;
    af->forward_speed = 0;
    af->reverse_speed = 0;
    af->jump_speed = 0;
    af->fall_speed = 0;
    af->unknown_c = 0;
    return af;
}

void sd_af_set_move(sd_af_file *af, int index, sd_move *move) {
    if(index < 0 || index >= MAX_AF_MOVES || af == NULL)
        return;

    if(af->moves[index] != NULL) {
        sd_move_delete(af->moves[index]);
    }
    af->moves[index] = move;
}

sd_move* sd_af_get_move(sd_af_file *af, int index) {
    if(index < 0 || index >= MAX_AF_MOVES || af == NULL)
        return NULL;

    return af->moves[index];
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
        if(moveno >= MAX_AF_MOVES || !sd_reader_ok(r)) {
            break;
        }

        // Read move
        af->moves[moveno] = sd_move_create();
        int ret = sd_move_load(r, af->moves[moveno]);
        if(ret != 0) {
            return ret;
        }
    }

    // Read soundtable
    sd_read_buf(r, af->soundtable, 30);

    // Fix missing sprites
    sd_af_postprocess(af);

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
    sd_write_udword(w, af->endurance);
    sd_write_ubyte(w, af->unknown_b);
    sd_write_uword(w, af->power);
    sd_write_dword(w, af->forward_speed);
    sd_write_dword(w, af->reverse_speed);
    sd_write_dword(w, af->jump_speed);
    sd_write_dword(w, af->fall_speed);
    sd_write_uword(w, af->unknown_c);

    // Write animations
    for(uint8_t i = 0; i < MAX_AF_MOVES; i++) {
        if(af->moves[i]) {
            sd_write_ubyte(w, i);
            sd_move_save(w, af->moves[i]);
        }
    }

    // This marks the end of animations
    sd_write_ubyte(w, 250); 

    // Soundtable
    sd_write_buf(w, af->soundtable, 30);

    // All done!
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_af_delete(sd_af_file *af) {
    if(af == NULL)
        return;
    for(int i = 0; i < MAX_AF_MOVES; i++) {
        if(af->moves[i] != NULL) {
            sd_move_delete(af->moves[i]);
        }
    }
    free(af);
}
