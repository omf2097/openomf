#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/pic.h"

sd_pic_file* sd_pic_create() {
    sd_pic_file *f = malloc(sizeof(sd_pic_file));
    return f;
}

int sd_pic_load(sd_pic_file *pic, const char *filename) {
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    pic->photo_count = sd_read_dword(r);

    sd_reader_close(r);
    return SD_SUCCESS;
}

int sd_pic_save(sd_pic_file *pic, const char *filename) {
    return SD_FILE_OPEN_ERROR;
}

void sd_pic_delete(sd_pic_file *pic) {
    free(pic);
}

