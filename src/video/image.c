#include "video/image.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

// If png support is enabled, include header
#ifdef USE_PNG
#include <png.h>
#endif

#define CHECK_COORD_BOUNDS(n, x) n = (n >= x ? (x-1) : (n < 0 ? 0 : n))

typedef struct __attribute__ ((__packed__)) tga_header_t {
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
    img->data = malloc(w * h * 4);
    img->w = w;
    img->h = h;
    return 0;
}

void image_free(image *img) {
    free(img->data);
    img->data = NULL;
    img->w = 0;
    img->h = 0;
}

int image_create_from_sd(image *img, sd_rgba_image *sdimg) {
    image_create(img, sdimg->w, sdimg->h);
    memcpy(img->data, sdimg->data, sdimg->w * sdimg->h * 4);
    return 0;
}

void image_clear(image *img, color c) {
    image_filled_rect(img, 0, 0, img->w, img->h, c);
}

// Bresenham
void image_line(image *img, int x0, int y0, int x1, int y1, color c) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1; 
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;
 
    while(1) {
        image_set_pixel(img, x0, y0, c);
        if(x0 == x1 && y0 == y1) break;
        e2 = err;
        if(e2 > -dx) { err -= dy; x0 += sx; }
        if(e2 <  dy) { err += dx; y0 += sy; }
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
    image_line(img, x, y, x+w, y, c);
    image_line(img, x, y+h, x+w, y+h, c);
    image_line(img, x+w, y, x+w, y+h, c);
    image_line(img, x, y, x, y+h, c);
}

void image_rect_bevel(image *img, 
                      int x, int y, 
                      int w, int h, 
                      color ctop, color cright, 
                      color cbottom, color cleft) {
    image_line(img, x, y, x+w, y, ctop);
    image_line(img, x, y+h, x+w, y+h, cbottom);
    image_line(img, x+w, y, x+w, y+h, cright);
    image_line(img, x, y, x, y+h, cleft);
}

void image_filled_rect(image *img, int x, int y, int w, int h, color c) {
    for(int my = y; my < y+h; my++) {
        for(int mx = x; mx < x+w; mx++) {
            image_set_pixel(img, mx, my, c);
        }
    }
}

int image_write_tga(image *img, const char *filename) {
    // Open file
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL) {
        return 1; // error
    }
    
    // Write header
    tga_header header;
    header.id = 0;
    header.colormap = 0;
    header.type = 2;
    for(int i = 0; i < 5; i++) {
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
    for(int y = img->h-1; y >= 0; y--) {
        for(int x = 0; x < img->w; x++) {
            d = img->data + (y * img->w + x) * 4;
            fwrite(d+2, 1, 1, fp);
            fwrite(d+1, 1, 1, fp);
            fwrite(d+0, 1, 1, fp);
        }
    }
    
    // Free file
    fclose(fp);
    return 0; // Success
}

int image_supports_png() {
#ifdef USE_PNG
    return 1;
#else
    return 0;
#endif
}

int image_write_png(image *img, const char *filename) {
#ifdef USE_PNG
    // Open file
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL) {
        return 1;
    }

    // PNG stuff
    png_structp png_ptr;
    png_infop info_ptr;
    
    // Get row pointers
    char *rows[img->h];
    for(int y = 0; y < img->h; y++) {
        rows[y] = malloc(img->w * 3);
        for(int x = 0; x < img->w; x++) {
            rows[y][x * 3 + 0] = img->data[y * img->w + x * 4 + 0];
            rows[y][x * 3 + 1] = img->data[y * img->w + x * 4 + 1];
            rows[y][x * 3 + 2] = img->data[y * img->w + x * 4 + 2];
        }
    }
    
    // Init
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);
    
    // Write header. RGB, 8bits per channel
    setjmp(png_jmpbuf(png_ptr));
    png_set_IHDR(png_ptr, 
                 info_ptr, 
                 img->w, 
                 img->h,
                 8, 
                 PNG_COLOR_TYPE_RGB, 
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, 
                 PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    
    // Write data
    setjmp(png_jmpbuf(png_ptr));
    png_write_image(png_ptr, (void*)rows);
    
    // End
    setjmp(png_jmpbuf(png_ptr));
    png_write_end(png_ptr, NULL);

    // Free memory
    for(int i = 0; i < img->h; i++) {
        free(rows[i]);
    }

    // Free file
    fclose(fp);
    return 0; // Success
#else
    return 1; // PNG not supported, report failure
#endif
}
