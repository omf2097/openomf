#include "bk.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "internal/animation.h"
#include <stdlib.h>

bk_file* bk_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    bk_file *data = malloc(sizeof(bk_file));

    // Header
    data->file_id = sd_read_udword(r);
    data->unknown_a = sd_read_ubyte(r);
    data->img_w = sd_read_uword(r);
    data->img_h = sd_read_uword(r);

    // Read animations
    int8_t animno = 0;
    while(1) {
        sd_skip(r, 4);
        animno = sd_read_ubyte(r);
        if(animno >= 50 || !sd_reader_ok(r)) {
            break;
        }

        // TODO: Parse animation header here
    }

    // TODO: Read VGA Image
    // TODO: Read palettes
    // TODO: Read footer

    // Close & return
    sd_reader_close(r);
    return data;
}

int bk_save(const char* filename, bk_file *data) {
    return 0;
}

void bk_destroy(bk_file *data) {
    free(data);
}
