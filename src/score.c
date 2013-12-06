#include <stdlib.h>
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/score.h"

sd_score* sd_score_create() {
    return malloc(sizeof(sd_score));
}

void sd_score_delete(sd_score *score) {
    free(score);
}

int sd_score_load(sd_score *score, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(r) {
        if(!sd_read_buf(r, (char*)score, sizeof(sd_score))) {
            return SD_FILE_PARSE_ERROR;
        }
        if(!sd_reader_ok(r)) {
            return SD_FILE_PARSE_ERROR;
        }
        sd_reader_close(r);
    } else {
        return SD_FILE_OPEN_ERROR;
    }
    return SD_SUCCESS;
}

void sd_score_save(sd_score *score, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(w) {
        sd_write_buf(w, (char*)score, sizeof(sd_score));
        sd_writer_close(w);
    }
}
