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
