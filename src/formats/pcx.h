#ifndef PCX_H
#define PCX_H

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

    uint8_t palette[48];
    uint8_t reserved;
    uint8_t color_planes;

    uint16_t bytes_per_plane_line;
    uint16_t palette_info;

    uint16_t hor_scr_size;
    uint16_t ver_scr_size;

    // After the headers here, there is 54 bytes of padding.

    uint8_t image[200][320];
} pcx_file;

int pcx_load(pcx_file *pcx, const char *filename);

#endif // PCX_H
