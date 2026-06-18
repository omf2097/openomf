#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "formats/af.h"
#include "formats/animation.h"
#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/move.h"
#include "utils/allocator.h"

int sd_af_create(sd_af_file *af) {
    assert(af != NULL);

    // Clear everything
    memset(af, 0, sizeof(sd_af_file));
    return SD_SUCCESS;
}

int sd_af_copy(sd_af_file *dst, const sd_af_file *src) {
    int ret;
    assert(dst != NULL);
    assert(src != NULL);

    // Clear destination
    memset(dst, 0, sizeof(sd_af_file));

    // Copy the basic stuff
    dst->fighter_id = src->fighter_id;
    dst->exec_window = src->exec_window;
    dst->endurance = src->endurance;
    dst->upwards_jump_frame_limit = src->upwards_jump_frame_limit;
    dst->health = src->health;
    dst->forward_speed = src->forward_speed;
    dst->reverse_speed = src->reverse_speed;
    dst->jump_speed = src->jump_speed;
    dst->fall_speed = src->fall_speed;
    dst->version_1 = src->version_1;
    dst->ai_projectile_y_threshold = src->ai_projectile_y_threshold;

    // Copy sound_table
    memcpy(dst->sound_table, src->sound_table, sizeof(src->sound_table));

    // Copy move animations
    for(int i = 0; i < MAX_AF_MOVES; i++) {
        if(src->moves[i] != NULL) {
            dst->moves[i] = omf_calloc(1, sizeof(sd_move));
            if((ret = sd_move_copy(dst->moves[i], src->moves[i])) != SD_SUCCESS) {
                return ret;
            }
        }
    }

    return SD_SUCCESS;
}

int sd_af_set_move(sd_af_file *af, int index, const sd_move *move) {
    int ret;
    assert(af != NULL);
    if(index < 0 || index >= MAX_AF_MOVES) {
        return SD_INVALID_INPUT;
    }

    if(af->moves[index] != NULL) {
        sd_move_free(af->moves[index]);
        omf_free(af->moves[index]);
    }
    if(move == NULL) {
        return SD_SUCCESS;
    }
    af->moves[index] = omf_calloc(1, sizeof(sd_move));
    if((ret = sd_move_copy(af->moves[index], move)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_move *sd_af_get_move(sd_af_file *af, int index) {
    if(af == NULL || index < 0 || index >= MAX_AF_MOVES) {
        return NULL;
    }
    return af->moves[index];
}

void sd_af_postprocess(sd_af_file *af) {
    char *table[1000] = {0}; // temporary lookup table
    // fix NULL pointers for any 'missing' sprites
    for(int i = 0; i < 70; i++) {
        if(af->moves[i] != NULL) {
            sd_animation *anim = af->moves[i]->animation;
            int sprite_count = sd_animation_get_sprite_count(anim);
            for(int j = 0; j < sprite_count; j++) {
                sd_sprite *sprite = sd_animation_get_sprite(anim, j);
                if(sprite->missing > 0) {
                    if(table[sprite->index]) {
                        sprite->data = table[sprite->index];
                    }
                } else {
                    table[sprite->index] = sprite->data;
                }
            }
        }
    }
}

int sd_af_load(sd_af_file *af, const path *filename) {
    int ret = SD_SUCCESS;
    uint8_t moveno = 0;
    sd_reader *r;

    // Initialize reader
    if(!(r = sd_reader_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    af->fighter_id = sd_read_uword(r);
    af->exec_window = sd_read_uword(r); // Always 10
    af->endurance = sd_read_udword(r) * 1.0f;
    af->upwards_jump_frame_limit = sd_read_ubyte(r); // Always 1 or 2
    af->health = sd_read_uword(r);
    af->forward_speed = sd_read_dword(r) / 256.0f;
    af->reverse_speed = sd_read_dword(r) / 256.0f;
    af->jump_speed = sd_read_dword(r) / 256.0f;
    af->fall_speed = sd_read_dword(r) / 256.0f;
    af->version_1 = sd_read_ubyte(r);                 // Always 0x32 ?
    af->ai_projectile_y_threshold = sd_read_ubyte(r); // Always 0x14 ?

    // Read animations
    while(1) {
        moveno = sd_read_ubyte(r);
        if(moveno >= MAX_AF_MOVES || !sd_reader_ok(r)) {
            break;
        }

        // Read move
        af->moves[moveno] = omf_calloc(1, sizeof(sd_move));
        if((ret = sd_move_create(af->moves[moveno])) != SD_SUCCESS) {
            goto cleanup;
        }
        if((ret = sd_move_load(r, af->moves[moveno])) != SD_SUCCESS) {
            goto cleanup;
        }
    }

    sd_read_buf(r, af->sound_table, 30);

    // Fix missing sprites
    sd_af_postprocess(af);

cleanup:
    // Close & return
    sd_reader_close(r);
    return ret;
}

int sd_af_save(const sd_af_file *af, const path *filename) {
    int ret;
    sd_writer *w;

    if(!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Header
    sd_write_uword(w, af->fighter_id);
    sd_write_uword(w, af->exec_window);
    sd_write_udword(w, (int)(af->endurance * 256));
    sd_write_ubyte(w, af->upwards_jump_frame_limit);
    sd_write_uword(w, af->health);
    sd_write_dword(w, (int)(af->forward_speed * 256));
    sd_write_dword(w, (int)(af->reverse_speed * 256));
    sd_write_dword(w, (int)(af->jump_speed * 256));
    sd_write_dword(w, (int)(af->fall_speed * 256));
    sd_write_ubyte(w, af->version_1);
    sd_write_ubyte(w, af->ai_projectile_y_threshold);

    // Write animations
    for(uint8_t i = 0; i < MAX_AF_MOVES; i++) {
        if(af->moves[i] != NULL) {
            sd_write_ubyte(w, i);
            if((ret = sd_move_save(w, af->moves[i])) != SD_SUCCESS) {
                sd_writer_close(w);
                return ret;
            }
        }
    }

    // This marks the end of animations
    sd_write_ubyte(w, 250);

    sd_write_buf(w, af->sound_table, 30);

    // All done!
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_af_free(sd_af_file *af) {
    if(af == NULL) {
        return;
    }
    for(int i = 0; i < MAX_AF_MOVES; i++) {
        if(af->moves[i] != NULL) {
            sd_move_free(af->moves[i]);
            omf_free(af->moves[i]);
        }
    }
}
