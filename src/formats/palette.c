#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "formats/error.h"
#include "formats/palette.h"

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

int sd_palette_to_gimp_palette(const sd_palette *palette, const char *filename) {
    sd_writer *w;
    const unsigned char *d;
    unsigned char r,g,b;
    int i;

    if(palette == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    if(!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    sd_write_fprintf(w, "GIMP Palette\n");
    sd_write_fprintf(w, "Name: %s\n", filename);
    sd_write_fprintf(w, "#\n");
    for(i = 0; i < 255; i++) {
        d = palette->data[i];
        r = d[0] & 0xff;
        g = d[1] & 0xff;
        b = d[2] & 0xff;
        sd_write_fprintf(w, "%3u %3u %3u\n", r, g, b);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_palette_from_gimp_palette(sd_palette *palette, const char *filename) {
    sd_reader *rd;
    char tmp[128];
    int i;

    if(palette == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    if(!(rd = sd_reader_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Read and match header
    if(!sd_match(rd, "GIMP Palette\n", 13)) {
        sd_reader_close(rd);
        return SD_FILE_INVALID_TYPE;
    }
    while(1) {
        sd_read_line(rd, tmp, 128);
        if(!sd_reader_ok(rd)) {
            sd_reader_close(rd);
            return SD_FILE_INVALID_TYPE;
        }
        if(tmp[0] == '#') {
            break;
        }
    }

    // Read data
    for(i = 0; i < 255; i++) {
        sd_read_line(rd, tmp, 128);
        tmp[3] = 0;
        tmp[7] = 0;
        tmp[11] = 0;
        palette->data[i][0] = atoi(tmp + 0);
        palette->data[i][1] = atoi(tmp + 4);
        palette->data[i][2] = atoi(tmp + 8);
    }

    sd_reader_close(rd);
    return SD_SUCCESS;
}

int sd_palette_load_range(sd_reader *reader, sd_palette *palette, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        sd_read_buf(reader, d, 3);
        palette->data[i][0] = ((d[0] << 2) | ((d[0] & 0x30) >> 4));
        palette->data[i][1] = ((d[1] << 2) | ((d[1] & 0x30) >> 4));
        palette->data[i][2] = ((d[2] << 2) | ((d[2] & 0x30) >> 4));
    }
    return SD_SUCCESS;
}

int sd_palette_mload_range(sd_mreader *reader, sd_palette *palette, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        sd_mread_buf(reader, d, 3);
        palette->data[i][0] = ((d[0] << 2) | ((d[0] & 0x30) >> 4));
        palette->data[i][1] = ((d[1] << 2) | ((d[1] & 0x30) >> 4));
        palette->data[i][2] = ((d[2] << 2) | ((d[2] & 0x30) >> 4));
    }
    return SD_SUCCESS;
}

int sd_palette_load(sd_reader *reader, sd_palette *palette) {
    sd_palette_load_range(reader, palette, 0, 256);
    sd_read_buf(reader, (char*)palette->remaps, 19*256);
    return SD_SUCCESS;
}

void sd_palette_save_range(sd_writer *writer, const sd_palette *palette, int index_start, int index_count) {
    const unsigned char *d;
    for(int i = index_start; i < index_start + index_count; i++) {
        d = palette->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[2] & 0xff) >> 2);
    }
}

void sd_palette_msave_range(sd_mwriter *writer, const sd_palette *palette, int index_start, int index_count) {
    const unsigned char *d;
    for(int i = index_start; i < index_start + index_count; i++) {
        d = palette->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_mwrite_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_mwrite_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_mwrite_ubyte(writer, (d[2] & 0xff) >> 2);
    }
}

void sd_palette_save(sd_writer *writer, const sd_palette *palette) {
    sd_palette_save_range(writer, palette, 0, 256);
    sd_write_buf(writer, (char*)palette->remaps, 19*256);
}
