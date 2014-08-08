#include <stdint.h>
#include <string.h>
#include "shadowdive/error.h"
#include "shadowdive/palette.h"

int sd_palette_create(sd_palette *pal) {
    if(pal == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(pal, 0, sizeof(sd_palette));
    return SD_SUCCESS;
}

void sd_palette_free(sd_palette *pal) {}

unsigned char sd_palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const sd_palette *pal) {
    for(unsigned int i = 0; i < 256; i++) {
        uint8_t red = pal->data[i][0] & 0xff;
        uint8_t green = pal->data[i][1] & 0xff;
        uint8_t blue = pal->data[i][2] & 0xff;
        if(red == r && blue == b && green == g) {
            return i;
        }
    }
    return 0;
}

int sd_palette_to_gimp_palette(const char *filename, const sd_palette *palette) {
    sd_writer *w;;
    const unsigned char *d;

    if(!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    sd_write_fprintf(w, "GIMP Palette\n");
    sd_write_fprintf(w, "Name: %s\n", filename);
    sd_write_fprintf(w, "#\n");
    for(int i = 0; i < 255; i++) {
        d = palette->data[i];
        unsigned char r = d[0] & 0xff;
        unsigned char g = d[1] & 0xff;
        unsigned char b = d[2] & 0xff;
        sd_write_fprintf(w, "%3u %3u %3u\n", r, g, b);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_palette_load(sd_reader *reader, sd_palette *palette) {
    char d[3];
    for(int i = 0; i < 256; i++) {
        sd_read_buf(reader, d, 3);
        palette->data[i][0] = ((d[0] << 2) | ((d[0] & 0x30) >> 4));
        palette->data[i][1] = ((d[1] << 2) | ((d[1] & 0x30) >> 4));
        palette->data[i][2] = ((d[2] << 2) | ((d[2] & 0x30) >> 4));
    }
    sd_read_buf(reader, (char*)palette->remaps, 19*256);
    return SD_SUCCESS;
}

void sd_palette_save(sd_writer *writer, const sd_palette *palette) {
    const unsigned char *d;
    for(int i = 0; i < 256; i++) {
        d = palette->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[2] & 0xff) >> 2);
    }
    sd_write_buf(writer, (char*)palette->remaps, 19*256);
}
