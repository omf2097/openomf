#include "formats/palette.h"
#include "formats/altpal.h"
#include "formats/error.h"
#include "utils/allocator.h"
#include "video/vga_state.h"
#include <stdlib.h>
#include <string.h>

#define COLOR_6TO8(color) ((color << 2) | ((color & 0x30) >> 4))
#define COLOR_8TO6(color) ((color >> 2) & 0x3F)

// Inserted to range 250 - 255.
static const vga_color menu_colors[6] = {
    {0,  0,  42}, // 0, 0, 170
    {0,  0,  60}, // 0, 0, 242
    {0,  0,  22}, // 0, 0, 89
    {0,  63, 0 }, // 0, 255, 0
    {0,  30, 0 }, // 0, 121, 0
    {20, 63, 20}, // 80, 255, 80
};

static const vga_color pulse_colors[9] = {
    {0,  30, 0 },
    {2,  34, 2 },
    {5,  38, 5 },
    {7,  42, 7 },
    {10, 46, 10},
    {12, 50, 12},
    {15, 54, 15},
    {17, 60, 17},
    {20, 63, 20},
};

unsigned char palette_resolve_color(uint8_t r, uint8_t g, uint8_t b, const vga_palette *pal) {
    for(unsigned int i = 0; i < 256; i++) {
        uint8_t red = pal->colors[i].r & 0xff;
        uint8_t green = pal->colors[i].g & 0xff;
        uint8_t blue = pal->colors[i].b & 0xff;
        if(red == r && blue == b && green == g) {
            return i;
        }
    }
    return 0;
}

int palette_to_gimp_palette(const vga_palette *pal, const char *filename) {
    sd_writer *w;
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
        r = pal->colors[i].r & 0xff;
        g = pal->colors[i].g & 0xff;
        b = pal->colors[i].b & 0xff;
        sd_write_fprintf(w, "%3u %3u %3u\n", r, g, b);
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int palette_from_gimp_palette(vga_palette *pal, const char *filename) {
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
        pal->colors[i].r = atoi(tmp + 0);
        pal->colors[i].g = atoi(tmp + 4);
        pal->colors[i].b = atoi(tmp + 8);
    }

    sd_reader_close(rd);
    return SD_SUCCESS;
}

void palette_set_menu_colors(void) {
    // Set the default menu colors. These are always set for the default (0) palette.
    vga_palette pal;
    for(int i = 0; i < 6; i++) {
        pal.colors[i].r = COLOR_6TO8(menu_colors[i].r);
        pal.colors[i].g = COLOR_6TO8(menu_colors[i].g);
        pal.colors[i].b = COLOR_6TO8(menu_colors[i].b);
    }
    vga_state_set_base_palette_from_range(&pal, 250, 0, 6);
}

void palette_pulse_menu_colors(int tick) {
    int i = tick % 16;
    if(i > 8) {
        i = 16 - i;
    }
    vga_color c = {
        COLOR_6TO8(pulse_colors[i].r),
        COLOR_6TO8(pulse_colors[i].g),
        COLOR_6TO8(pulse_colors[i].b),
    };
    vga_state_set_base_palette_index(255, &c);
}

int palette_load_range(sd_reader *reader, vga_palette *pal, int index_start, int index_count) {
    assert(index_start + index_count <= 256);
    vga_color d;
    for(int i = index_start; i < index_start + index_count; i++) {
        sd_read_buf(reader, (char *)&d, 3);
        pal->colors[i].r = COLOR_6TO8(d.r);
        pal->colors[i].g = COLOR_6TO8(d.g);
        pal->colors[i].b = COLOR_6TO8(d.b);
    }
    return SD_SUCCESS;
}

int palette_mload_range(memreader *reader, vga_palette *pal, int index_start, int index_count) {
    assert(index_start + index_count <= 256);
    vga_color d;
    for(int i = index_start; i < index_start + index_count; i++) {
        memread_buf(reader, (char *)&d, 3);
        pal->colors[i].r = COLOR_6TO8(d.r);
        pal->colors[i].g = COLOR_6TO8(d.g);
        pal->colors[i].b = COLOR_6TO8(d.b);
    }
    return SD_SUCCESS;
}

int palette_load(sd_reader *reader, vga_palette *pal) {
    palette_load_range(reader, pal, 0, 256);
    return SD_SUCCESS;
}

int palette_remaps_load(sd_reader *reader, vga_remap_tables *remaps) {
    sd_read_buf(reader, (char *)remaps, sizeof(vga_remap_tables));
    return SD_SUCCESS;
}

void palette_save_range(sd_writer *writer, const vga_palette *pal, int index_start, int index_count) {
    for(int i = index_start; i < index_start + index_count; i++) {
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        sd_write_ubyte(writer, COLOR_8TO6(pal->colors[i].r));
        sd_write_ubyte(writer, COLOR_8TO6(pal->colors[i].g));
        sd_write_ubyte(writer, COLOR_8TO6(pal->colors[i].b));
    }
}

void palette_msave_range(memwriter *writer, const vga_palette *pal, int index_start, int index_count) {
    for(int i = index_start; i < index_start + index_count; i++) {
        // for some reason, we need to mask off the high bits or the bitshift doesn't work
        memwrite_ubyte(writer, COLOR_8TO6(pal->colors[i].r));
        memwrite_ubyte(writer, COLOR_8TO6(pal->colors[i].g));
        memwrite_ubyte(writer, COLOR_8TO6(pal->colors[i].b));
    }
}

void palette_save(sd_writer *writer, const vga_palette *pal) {
    palette_save_range(writer, pal, 0, 256);
}

void palette_remaps_save(sd_writer *writer, const vga_remap_tables *remaps) {
    if(remaps) {
        sd_write_buf(writer, (char *)remaps, sizeof(vga_remap_tables));
    }
}

void palette_load_player_colors(vga_palette *src, int player) {
    // only load 47 palette colors, skipping the first one
    // because that seems to be ignored by the original
    int dst_offset = (player * 48) + 1;
    vga_state_set_base_palette_from_range(src, dst_offset, 1, 47);
}

void palette_load_altpal_player_color(vga_palette *dst, int player, int src_color, int dst_color) {
    int dst_index = dst_color * 16 + player * 48;
    int src_index = src_color * 16;
    vga_color tmp = dst->colors[0];
    if(altpals) {
        memcpy(&dst->colors[dst_index], &altpals->palettes[0].colors[src_index], 16 * 3);
    }
    dst->colors[0] = tmp;
}

void palette_set_player_color(int player, int src_color, int dst_color) {
    int dst_index = dst_color * 16 + player * 48;
    int src_index = src_color * 16;
    vga_palette pal;
    vga_palette_init(&pal);
    palette_load_altpal_player_color(&pal, player, src_color, dst_color);
    vga_state_set_base_palette_from_range(&pal, dst_index, src_index, 16 * 3);
}

void palette_set_player_expanded_color(vga_palette *src) {
    // expand the player 1 colors, which are 3 shades of 16 colors
    // into 3 shades of 32 colors

    vga_palette tmp;
    vga_color lower, upper;
    for(int j = 0; j < 3; j++) {
        for(int i = 0; i < 32; ++i) {
            float position = (float)i * 15.0f / 31.0f;
            int lower_index = (int)position;
            float t = position - lower_index;
            int upper_index = lower_index + 1;
            if(upper_index >= 16) {
                // don't go beyond the last color in this HAR color
                upper_index = 15;
            }

            lower = src->colors[lower_index + (j * 16)];
            upper = src->colors[upper_index + (j * 16)];

            tmp.colors[i + (j * 32)].r = (uint8_t)((1.0f - t) * lower.r + t * upper.r + 0.5f);
            tmp.colors[i + (j * 32)].g = (uint8_t)((1.0f - t) * lower.g + t * upper.g + 0.5f);
            tmp.colors[i + (j * 32)].b = (uint8_t)((1.0f - t) * lower.b + t * upper.b + 0.5f);
        }
    }
    vga_state_set_base_palette_from_range(&tmp, 1, 1, 96);
}

void palette_copy(vga_palette *dst, const vga_palette *src, int index_start, int index_count) {
    memcpy(&dst->colors[index_start], &src->colors[index_start], index_count * 3);
}
