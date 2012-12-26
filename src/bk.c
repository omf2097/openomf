#include "bk.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include <stdlib.h>

bk_file* bk_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    bk_file *bk = malloc(sizeof(bk_file));

    // Header
    bk->file_id = sd_read_udword(r);
    bk->unknown_a = sd_read_ubyte(r);
    bk->img_w = sd_read_uword(r);
    bk->img_h = sd_read_uword(r);

    // Read animations
    int8_t animno = 0;
    while(1) {
        sd_skip(r, 4);
        animno = sd_read_ubyte(r);
        if(animno >= 50 || !sd_reader_ok(r)) {
            break;
        }

        // TODO: Parse animation here
    }

    // Read background image
    bk->background = sd_vga_image_create(bk->img_w, bk->img_h);
    sd_read_buf(r, bk->background->data, bk->img_h * bk->img_w);

    // Read palettes
    uint8_t num_palettes = sd_read_ubyte(r);
    bk->palettes = malloc(num_palettes * sizeof(palette));
    for(uint8_t i = 0; i < num_palettes; i++) {
        sd_read_buf(r, (char*)bk->palettes[i]->data, 256*3);
        sd_read_buf(r, (char*)bk->palettes[i]->remaps, 19*256);
    }

    // Read footer
    sd_read_buf(r, bk->footer, 30);

    // Close & return
    sd_reader_close(r);
    return bk;
}

int bk_save(const char* filename, bk_file *bk) {
    return 0;
}

void bk_destroy(bk_file *bk) {
    sd_vga_image_delete(bk->background);
    free(bk->palettes);
    free(bk);
}
