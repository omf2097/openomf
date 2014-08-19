#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/rec.h"

int sd_rec_create(sd_rec_file *rec) {
    if(rec == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(rec, 0, sizeof(sd_rec_file));
    return SD_SUCCESS;
}

void sd_rec_free(sd_rec_file *rec) {
    if(rec == NULL) return;
    for(int i = 0; i < 2; i++) {
        if(rec->pilots[i]) {
            sd_pilot_free(rec->pilots[i]);
            free(rec->pilots[i]);
        }
    }
    if(rec->moves) {
        free(rec->moves);
    }
    if(rec->raw) {
        free(rec->raw);
    }
}

int sd_rec_load(sd_rec_file *rec, const char *file) {
    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure we have at least this much data
    if(sd_reader_filesize(r) < 1224) {
        goto error_0;
    }

    // Read pilot data
    for(int i = 0; i < 2; i++) {
        rec->pilots[i] = malloc(sizeof(sd_pilot));
        sd_pilot_create(rec->pilots[i]);
        sd_pilot_load(r, rec->pilots[i]);
        sd_skip(r, 168); // This contains empty palette and psrite etc. Just skip.
    }

    // Scores
    for(int i = 0; i < 2; i++)
        rec->scores[i] = sd_read_udword(r);

    // Other flags
    rec->unknown_a = sd_read_byte(r);
    rec->unknown_b = sd_read_byte(r);
    rec->unknown_c = sd_read_byte(r);
    rec->unknown_d = sd_read_word(r);
    rec->unknown_e = sd_read_word(r);
    rec->unknown_f = sd_read_word(r);
    rec->unknown_g = sd_read_word(r);
    rec->unknown_h = sd_read_word(r);
    rec->unknown_i = sd_read_word(r);
    rec->unknown_j = sd_read_word(r);
    rec->unknown_k = sd_read_word(r);
    rec->unknown_l = sd_read_dword(r);
    rec->unknown_m = sd_read_byte(r);

    size_t pos = sd_reader_pos(r);
    size_t len = sd_reader_filesize(r);
    size_t rsize = len - pos;

    // Read rest of the raw data
    rec->rawsize = rsize;
    rec->raw = malloc(rec->rawsize);
    sd_read_buf(r, rec->raw, rsize);

    // Set position back to the start of the move data, and read it to a pile of structs
    sd_reader_set(r, pos);
    rec->move_count = rsize / 7;
    rec->moves = malloc(rec->move_count * sizeof(sd_rec_move));

    for(int i = 0; i < rec->move_count; i++) {
        rec->moves[i].tick = sd_read_udword(r);
        rec->moves[i].extra = sd_read_ubyte(r);
        rec->moves[i].player_id = sd_read_ubyte(r);
        rec->moves[i].action = sd_read_ubyte(r);
        if(rec->moves[i].extra > 2) {
            sd_read_buf(r, rec->moves[i].extra_data, 7);
            rec->move_count--;
        }
    }
    rec->moves = realloc(rec->moves, rec->move_count * sizeof(sd_rec_move));

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_0:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_rec_save(sd_rec_file *rec, const char *file) {
    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }
    return SD_FILE_OPEN_ERROR;
}