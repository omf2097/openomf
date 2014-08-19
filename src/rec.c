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

    // Read rest of the raw data
    rec->rawsize = sd_reader_filesize(r) - sd_reader_pos(r);
    rec->raw = malloc(rec->rawsize);
    sd_read_buf(r, rec->raw, rec->rawsize);

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