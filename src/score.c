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
    int err = SD_SUCCESS;
    sd_reader *r = sd_reader_open(filename);
    if(r) {
        for(int i = 0;i < sizeof(score->scores)/sizeof(score->scores[0]);i++) {
            for(int j = 0;j < sizeof(score->scores[0])/sizeof(score->scores[0][0]);j++) {
                sd_score_entry *e = &score->scores[i][j];
                e->score = sd_read_udword(r);
                if(!sd_read_buf(r, e->name, sizeof(e->name))) {
                    goto read_error;
                }
                uint32_t id = sd_read_udword(r);
                e->har_id   = id&0x3F;
                e->pilot_id = (id>>6)&0x3F;
                e->padding  = (id>>12)&0xFFFFF;
            }
        }
read_error:
        if(!sd_reader_ok(r)) {
            err = SD_FILE_PARSE_ERROR;
        }
        sd_reader_close(r);
    } else {
        err = SD_FILE_OPEN_ERROR;
    }
    return err;
}

void sd_score_save(sd_score *score, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(w) {
        for(int i = 0;i < sizeof(score->scores)/sizeof(score->scores[0]);i++) {
            for(int j = 0;j < sizeof(score->scores[0])/sizeof(score->scores[0][0]);j++) {
                sd_score_entry *e = &score->scores[i][j];
                sd_write_udword(w, e->score);
                if(!sd_write_buf(w, e->name, sizeof(e->name))) {
                    goto write_error;
                }
                sd_write_udword(w, (e->har_id&0x3F) | ((e->pilot_id&0x3F)<<6) | ((e->padding&0xFFFFF)<<12));
            }
        }
write_error:
        sd_writer_close(w);
    }
}
