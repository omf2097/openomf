#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/palette.h"
#include <stdint.h>

int sd_palette_load(sd_reader *reader, sd_palette *palette) {
    char d[3];
    for(int i = 0; i < 256; i++) {
        sd_read_buf(reader, d, 3);
        palette->data[i][0] = ((d[0] << 2) | (d[0] >> 4));
        palette->data[i][1] = ((d[1] << 2) | (d[1] >> 4));
        palette->data[i][2] = ((d[2] << 2) | (d[2] >> 4));
    }
    sd_read_buf(reader, (char*)palette->remaps, 19*256);
    return SD_SUCCESS;
}

unsigned char sd_palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, sd_palette *pal) {
    /*for(unsigned int i = 255; i >= 0; i--) {*/
    for(unsigned int i = 0; i < 256; i++) {
        uint8_t red = pal->data[i][0] & 0xff;
        uint8_t green = pal->data[i][1] & 0xff;
        uint8_t blue = pal->data[i][2] & 0xff;
        if (red == r && blue == b && green == g) {
            return i;
        }
    }
    return 0;
}

void sd_palette_to_gimp_palette(char *filename, sd_palette *palette) {
    sd_writer *writer = sd_writer_open(filename);
    unsigned char *d;

    sd_write_fprintf(writer, "GIMP Palette\n");
    sd_write_fprintf(writer, "Name: %s\n", filename);
    sd_write_fprintf(writer, "#\n");
    for (int i = 0; i < 255; i++) {
        d = palette->data[i];
        unsigned char r = d[0] & 0xff;
        unsigned char g = d[1] & 0xff;
        unsigned char b = d[2] & 0xff;
        sd_write_fprintf(writer, "%3u %3u %3u\n", r, g, b);
    }
    return;
}

void sd_palette_save(sd_writer *writer, sd_palette *palette) {
    unsigned char *d;
    for(int i = 0; i < 256; i++) {
        d = palette->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[2] & 0xff) >> 2);
    }
    sd_write_buf(writer, (char*)palette->remaps, 19*256);
}
