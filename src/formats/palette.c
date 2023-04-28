#include "formats/palette.h"
#include "formats/altpal.h"
#include "formats/error.h"
#include "utils/allocator.h"
#include "video/color.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int palette_create(palette *pal) {
    if(pal == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(pal, 0, sizeof(palette));
    return SD_SUCCESS;
}

void palette_free(palette *pal) {
}

unsigned char palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const palette *pal) {
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

color palette_lookup_color(uint8_t i, const palette *pal) {
    uint8_t red = pal->data[i][0] & 0xff;
    uint8_t green = pal->data[i][1] & 0xff;
    uint8_t blue = pal->data[i][2] & 0xff;
    return color_create(red, green, blue, 255);
}

int palette_to_gimp_palette(const palette *pal, const char *filename) {
    sd_writer *w;
    const unsigned char *d;
    unsigned char r, g, b;
    int i;

    if(pal == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }

    if(!(w = sd_writer_open(filename))) {
        return SD_FILE_OPEN_ERROR;
    }

    sd_write_fprintf(w, "GIMP Palette\n");
    sd_write_fprintf(w, "Name: %s\n", filename);
    sd_write_fprintf(w, "#\n");
    for(i = 0; i < 255; i++) {
        d = pal->data[i];
        r = d[0] & 0xff;
        g = d[1] & 0xff;
        b = d[2] & 0xff;
        sd_write_fprintf(w, "%3u %3u %3u\n", r, g, b);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int palette_from_gimp_palette(palette *pal, const char *filename) {
    sd_reader *rd;
    char tmp[128];
    int i;

    if(pal == NULL || filename == NULL) {
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
        pal->data[i][0] = atoi(tmp + 0);
        pal->data[i][1] = atoi(tmp + 4);
        pal->data[i][2] = atoi(tmp + 8);
    }

    sd_reader_close(rd);
    return SD_SUCCESS;
}

int palette_load_range(sd_reader *reader, palette *pal, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        sd_read_buf(reader, d, 3);
        pal->data[i][0] = ((d[0] << 2) | ((d[0] & 0x30) >> 4));
        pal->data[i][1] = ((d[1] << 2) | ((d[1] & 0x30) >> 4));
        pal->data[i][2] = ((d[2] << 2) | ((d[2] & 0x30) >> 4));
    }
    return SD_SUCCESS;
}

int palette_mload_range(memreader *reader, palette *pal, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        memread_buf(reader, d, 3);
        pal->data[i][0] = ((d[0] << 2) | ((d[0] & 0x30) >> 4));
        pal->data[i][1] = ((d[1] << 2) | ((d[1] & 0x30) >> 4));
        pal->data[i][2] = ((d[2] << 2) | ((d[2] & 0x30) >> 4));
    }
    return SD_SUCCESS;
}

int palette_load(sd_reader *reader, palette *pal) {
    palette_load_range(reader, pal, 0, 256);
    sd_read_buf(reader, (char *)pal->remaps, 19 * 256);
    return SD_SUCCESS;
}

void palette_save_range(sd_writer *writer, const palette *pal, int index_start, int index_count) {
    const unsigned char *d;
    for(int i = index_start; i < index_start + index_count; i++) {
        d = pal->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, (d[0] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[1] & 0xff) >> 2);
        sd_write_ubyte(writer, (d[2] & 0xff) >> 2);
    }
}

void palette_msave_range(memwriter *writer, const palette *pal, int index_start, int index_count) {
    const unsigned char *d;
    for(int i = index_start; i < index_start + index_count; i++) {
        d = pal->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        memwrite_ubyte(writer, (d[0] & 0xff) >> 2);
        memwrite_ubyte(writer, (d[1] & 0xff) >> 2);
        memwrite_ubyte(writer, (d[2] & 0xff) >> 2);
    }
}

void palette_save(sd_writer *writer, const palette *pal) {
    palette_save_range(writer, pal, 0, 256);
    sd_write_buf(writer, (char *)pal->remaps, 19 * 256);
}

void palette_load_player_colors(palette *dst, palette *src, int player) {
    // only load 47 palette colors, skipping the first one
    // because that seems to be ignored by the original
    int dstoff = (player * 48) + 1;
    memcpy(dst->data + dstoff, src->data + 1, 47 * 3);
}

void palette_set_player_color(palette *pal, int player, int srccolor, int dstcolor) {
    int dst = dstcolor * 16 + player * 48;
    int src = srccolor * 16;
    char iz[3];
    memcpy(iz, pal->data, 3);
    if(altpals) {
        memcpy(pal->data + dst, altpals->palettes[0].data + src, 16 * 3);
    }
    memcpy(pal->data, iz, 3);
}

void palette_copy(palette *dst, const palette *src, int index_start, int index_count) {
    for(int i = index_start; i < index_start + index_count; i++) {
        memcpy(dst->data[i], src->data[i], 3);
    }
}
