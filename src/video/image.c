#include <math.h>
#include <memory.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/image.h"

#define CHECK_COORD_BOUNDS(n, x) n = (n >= x ? (x - 1) : (n < 0 ? 0 : n))

typedef struct __attribute__((__packed__)) tga_header_t {
    uint8_t id;
    uint8_t colormap;
    uint8_t type;
    uint8_t colormap_spec[5];
    uint16_t origin_x;
    uint16_t origin_y;
    uint16_t width;
    uint16_t height;
    uint8_t depth;
    uint8_t descriptor;
} tga_header;

int image_create(image *img, int w, int h) {
    img->data = omf_calloc(1, w * h * 4);
    img->w = w;
    img->h = h;
    return 0;
}

void image_free(image *img) {
    omf_free(img->data);
    img->w = 0;
    img->h = 0;
}

int image_create_from_sd(image *img, sd_rgba_image *sdimg) {
    image_create(img, sdimg->w, sdimg->h);
    memcpy(img->data, sdimg->data, sdimg->w * sdimg->h * 4);
    return 0;
}

void image_clear(image *img, color c) { image_filled_rect(img, 0, 0, img->w, img->h, c); }

// Bresenham
void image_line(image *img, int x0, int y0, int x1, int y1, color c) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (1) {
        image_set_pixel(img, x0, y0, c);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void image_set_pixel(image *img, int x, int y, color c) {
    CHECK_COORD_BOUNDS(x, img->w);
    CHECK_COORD_BOUNDS(y, img->h);
    img->data[(y * img->w + x) * 4 + 0] = c.r;
    img->data[(y * img->w + x) * 4 + 1] = c.g;
    img->data[(y * img->w + x) * 4 + 2] = c.b;
    img->data[(y * img->w + x) * 4 + 3] = c.a;
}

void image_rect(image *img, int x, int y, int w, int h, color c) {
    image_line(img, x, y, x + w, y, c);
    image_line(img, x, y + h, x + w, y + h, c);
    image_line(img, x + w, y, x + w, y + h, c);
    image_line(img, x, y, x, y + h, c);
}

void image_rect_bevel(image *img, int x, int y, int w, int h, color ctop, color cright,
                      color cbottom, color cleft) {
    image_line(img, x, y, x + w, y, ctop);
    image_line(img, x, y + h, x + w, y + h, cbottom);
    image_line(img, x + w, y, x + w, y + h, cright);
    image_line(img, x, y, x, y + h, cleft);
}

void image_filled_rect(image *img, int x, int y, int w, int h, color c) {
    for (int my = y; my < y + h; my++) {
        for (int mx = x; mx < x + w; mx++) {
            image_set_pixel(img, mx, my, c);
        }
    }
}

int image_write_tga(image *img, const char *filename) {
    // Open file
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        return 1; // error
    }

    // Write header
    tga_header header;
    header.id = 0;
    header.colormap = 0;
    header.type = 2;
    for (int i = 0; i < 5; i++) {
        header.colormap_spec[i] = 0;
    }
    header.origin_x = 0;
    header.origin_y = 0;
    header.width = img->w;
    header.height = img->h;
    header.depth = 24;
    header.descriptor = 0;
    fwrite(&header, sizeof(tga_header), 1, fp);

    // Write data
    char *d = 0;
    for (int y = img->h - 1; y >= 0; y--) {
        for (int x = 0; x < img->w; x++) {
            d = img->data + (y * img->w + x) * 4;
            fwrite(d + 2, 1, 1, fp);
            fwrite(d + 1, 1, 1, fp);
            fwrite(d + 0, 1, 1, fp);
        }
    }

    // Free file
    fclose(fp);
    return 0; // Success
}

int image_write_png(image *img, const char *filename) {
    png_image out;
    memset(&out, 0, sizeof(out));
    out.version = PNG_IMAGE_VERSION;
    out.opaque = NULL;
    out.width = img->w;
    out.height = img->h;
    out.format = PNG_FORMAT_RGBA;
    out.flags = 0;
    out.colormap_entries = 0;

    png_image_write_to_file(&out, filename, 0, img->data, img->w * 4, NULL);

    if (PNG_IMAGE_FAILED(out)) {
        PERROR("Unable to write PNG file: %s", out.message);
        return 1;
    }
    return 0;
}
