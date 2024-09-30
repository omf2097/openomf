#include "formats/palette.h"
#include "formats/altpal.h"
#include "formats/error.h"
#include "utils/allocator.h"
#include <stdlib.h>
#include <string.h>

#define COLOR_6TO8(color) ((color * 255.0) / 63.0)
#define COLOR_8TO6(color) ((color * 63.0) / 255.0)

// Inserted to range 250 - 255.
static const int menu_colors[6][3] = {
    {0,  0,  42}, // 0, 0, 170
    {0,  0,  60}, // 0, 0, 242
    {0,  0,  22}, // 0, 0, 89
    {0,  63, 0 }, // 0, 255, 0
    {0,  30, 0 }, // 0, 121, 0
    {20, 63, 20}, // 80, 255, 80
};

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

void palette_set_menu_colors(palette *pal) {
    // Set the default menu colors. These are always set for the default (0) palette.
    for(int i = 0; i < 6; i++) {
        pal->data[250 + i][0] = COLOR_6TO8(menu_colors[i][0]);
        pal->data[250 + i][1] = COLOR_6TO8(menu_colors[i][1]);
        pal->data[250 + i][2] = COLOR_6TO8(menu_colors[i][2]);
    }
}

int palette_load_range(sd_reader *reader, palette *pal, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        sd_read_buf(reader, d, 3);
        pal->data[i][0] = COLOR_6TO8(d[0]);
        pal->data[i][1] = COLOR_6TO8(d[1]);
        pal->data[i][2] = COLOR_6TO8(d[2]);
    }
    return SD_SUCCESS;
}

int palette_mload_range(memreader *reader, palette *pal, int index_start, int index_count) {
    char d[3];
    for(int i = index_start; i < index_start + index_count; i++) {
        memread_buf(reader, d, 3);
        pal->data[i][0] = COLOR_6TO8(d[0]);
        pal->data[i][1] = COLOR_6TO8(d[1]);
        pal->data[i][2] = COLOR_6TO8(d[2]);
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
        sd_write_ubyte(writer, COLOR_8TO6(d[0]));
        sd_write_ubyte(writer, COLOR_8TO6(d[1]));
        sd_write_ubyte(writer, COLOR_8TO6(d[2]));
    }
}

void palette_msave_range(memwriter *writer, const palette *pal, int index_start, int index_count) {
    const unsigned char *d;
    for(int i = index_start; i < index_start + index_count; i++) {
        d = pal->data[i];
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        memwrite_ubyte(writer, COLOR_8TO6(d[0]));
        memwrite_ubyte(writer, COLOR_8TO6(d[1]));
        memwrite_ubyte(writer, COLOR_8TO6(d[2]));
    }
}

void palette_save(sd_writer *writer, const palette *pal) {
    palette_save_range(writer, pal, 0, 256);
    sd_write_buf(writer, (char *)pal->remaps, 19 * 256);
}

void palette_load_player_colors(palette *dst, palette *src, int player) {
    // only load 47 palette colors, skipping the first one
    // because that seems to be ignored by the original
    int dst_offset = (player * 48) + 1;
    memcpy(dst->data + dst_offset, src->data + 1, 47 * 3);
}

void palette_load_player_cutscene_colors(palette *dst, palette *src) {
    for(int i = 1; i < 48; i++) {
        memcpy(dst->data + i * 2, src->data + i, 3);
        memcpy(dst->data + i * 2 + 1, src->data + i, 3);
        // TODO do real palette interpolation
    }
}

void palette_set_player_color(palette *pal, int player, int src_color, int dst_color) {
    int dst = dst_color * 16 + player * 48;
    int src = src_color * 16;
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
