#include <string.h>

#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/score.h"
#include "utils/c_array_util.h"

int sd_score_create(sd_score *score) {
    if(score == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(score, 0, sizeof(sd_score));
    return SD_SUCCESS;
}

void sd_score_free(sd_score *score) {
}

int sd_score_load(sd_score *score, const char *filename) {
    if(score == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    for(unsigned i = 0; i < N_ELEMENTS(score->scores); i++) {
        for(unsigned j = 0; j < N_ELEMENTS(score->scores[0]); j++) {
            sd_score_entry *e = &score->scores[i][j];
            e->score = sd_read_udword(r);
            sd_read_buf(r, e->name, sizeof(e->name));
            uint32_t id = sd_read_udword(r);
            e->har_id = id & 0x3F;
            e->pilot_id = (id >> 6) & 0x3F;
            e->padding = (id >> 12) & 0xFFFFF;
            if(!sd_reader_ok(r)) {
                goto read_error;
            }
        }
    }

    sd_reader_close(r);
    return SD_SUCCESS;

read_error:
    sd_reader_close(r);
    return SD_FILE_PARSE_ERROR;
}

int sd_score_save(const sd_score *score, const char *filename) {
    if(score == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    for(unsigned i = 0; i < N_ELEMENTS(score->scores); i++) {
        for(unsigned j = 0; j < N_ELEMENTS(score->scores[0]); j++) {
            const sd_score_entry *e = &score->scores[i][j];
            sd_write_udword(w, e->score);
            sd_write_buf(w, e->name, sizeof(e->name));
            sd_write_udword(w, (e->har_id & 0x3F) | ((e->pilot_id & 0x3F) << 6) | ((e->padding & 0xFFFFF) << 12));
        }
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

const sd_score_entry *sd_score_get(const sd_score *score, int page, int entry_id) {
    if(score == NULL)
        return NULL;
    if(page < 0 || page >= SD_SCORE_PAGES) {
        return NULL;
    }
    if(entry_id < 0 || entry_id >= SD_SCORE_ENTRIES) {
        return NULL;
    }
    return &score->scores[page][entry_id];
}
