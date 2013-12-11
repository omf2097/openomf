#include "video/image.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

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
void image_line(image *img, unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, color c) {
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

void image_set_pixel(image *img, unsigned int x, unsigned int y, color c) {
    if(x >= img->w || y >= img->h) return;
    img->data[(y * img->w + x) * 4 + 0] = c.r;
    img->data[(y * img->w + x) * 4 + 1] = c.g;
    img->data[(y * img->w + x) * 4 + 2] = c.b;
    img->data[(y * img->w + x) * 4 + 3] = c.a;
}

void image_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c) {
    image_line(img, x, y, x+w, y, c);
    image_line(img, x, y+h, x+w, y+h, c);
    image_line(img, x+w, y, x+w, y+h, c);
    image_line(img, x, y, x, y+h, c);
}

void image_rect_bevel(image *img, 
                      unsigned int x, unsigned int y, 
                      unsigned int w, unsigned int h, 
                      color ctop, color cright, 
                      color cbottom, color cleft) {
    image_line(img, x, y, x+w, y, ctop);
    image_line(img, x, y+h, x+w, y+h, cbottom);
    image_line(img, x+w, y, x+w, y+h, cright);
    image_line(img, x, y, x, y+h, cleft);
}

void image_filled_rect(image *img, unsigned int x, unsigned int y, unsigned int w, unsigned int h, color c) {
    for(int my = y; my < y+h; my++) {
        for(int mx = x; mx < x+w; mx++) {
            image_set_pixel(img, mx, my, c);
        }
    }
}

void image_write_tga(image *img, const char *filename) {
    // Open file
    FILE *fp = fopen(filename, "wb");
    if(fp == NULL) {
        return;
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
    fwrite(img->data, img->w * img->h * 4, 1, fp);
    
    // Free file
    fclose(fp);
}
 