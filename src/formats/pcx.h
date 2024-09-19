#ifndef PCX_H
#define PCX_H

#include "formats/vga_image.h"
#include <stdint.h>

typedef struct {
    uint8_t manufacturer;
    uint8_t version;
    uint8_t encoding;
    uint8_t bits_per_plane;

    uint16_t window_x_min;
    uint16_t window_y_min;
    uint16_t window_x_max;
    uint16_t window_y_max;

    uint16_t horz_dpi;
    uint16_t vert_dpi;

    uint8_t header_palette[48];
    uint8_t reserved;
    uint8_t color_planes;

    uint16_t bytes_per_plane_line;
    uint16_t palette_info;

    uint16_t hor_scr_size;
    uint16_t ver_scr_size;

    // After the headers here, there is 54 bytes of padding.

    sd_vga_image image;
    palette palette;
} pcx_file;

typedef struct {
    uint16_t x;
    uint8_t y;
    uint8_t width;
} pcx_font_glyph;

typedef struct {
    pcx_file pcx;
    uint8_t glyph_height;
    uint8_t glyph_count;
    pcx_font_glyph glyphs[256];
} pcx_font;

int pcx_load(pcx_file *pcx, const char *filename);
int pcx_load_font(pcx_font *font, const char *filename);
void pcx_free(pcx_file *pcx);
void pcx_font_free(pcx_font *font);

#endif // PCX_H
