#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/chr.h"

sd_chr_file* sd_chr_create() {
    sd_chr_file *chr = malloc(sizeof(sd_chr_file));
    return chr;
}

int sd_chr_load(sd_chr_file *chr, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_chr_save(sd_chr_file *chr, const char *filename) {
    sd_writer *w = sd_writer_open(filename);
    if(!w) {
        return SD_FILE_OPEN_ERROR;
    }

    // TODO

    // Close & return
    sd_writer_close(w);
    return SD_SUCCESS;
}

void sd_chr_delete(sd_chr_file *chr) {
    free(chr);
}