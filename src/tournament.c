#include <stdlib.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/tournament.h"

sd_tournament_file* sd_tournament_create() {
    sd_tournament_file *trn = malloc(sizeof(sd_tournament_file));
    return trn;
}

int sd_tournament_load(sd_tournament_file *trn, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    trn->enemy_count = sd_read_word(r);
    trn->unknown_a = sd_read_word(r);
    trn->victory_text_offset = sd_read_dword(r);

    sd_read_buf(r, trn->bk_name, 14);

    trn->winnings_multiplier = sd_read_float(r);

    trn->unknown_b = sd_read_dword(r);

    trn->registration_free = sd_read_dword(r);
    trn->assumed_initial_value = sd_read_dword(r);
    trn->tournament_id = sd_read_dword(r);

    // Read offsets
    sd_reader_set(r, 300);
    for(int i = 0; i < trn->enemy_count + 1; i++) {
        trn->offset_list[i] = sd_read_dword(r);
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_tournament_save(sd_tournament_file *trn, const char *filename) {
    return SD_FILE_OPEN_ERROR;
}

void sd_tournament_delete(sd_tournament_file *trn) {
    free(trn);
}
