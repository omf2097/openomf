#include <stdlib.h>
#include <string.h>
#include "video/surface.h"

void surface_create(surface *sur, int type, int w, int h) {
    if(type == SURFACE_TYPE_RGBA) {
        sur->data = malloc(w*h*4);
    } else {
        sur->data = malloc(w*h);
    }
    sur->w = w;
    sur->h = h;
    sur->type = type;
}

void surface_create_from_data(surface *sur, int type, int w, int h, const char *src) {
    surface_create(sur, type, w, h);
    int size = w * h * ((type == SURFACE_TYPE_PALETTE) ? 1 : 4);
    memcpy(sur->data, src, size);
}

void surface_create_from_image(surface *sur, image *img) {
    surface_create_from_data(sur, SURFACE_TYPE_RGBA, img->w, img->h, img->data);
}

void surface_free(surface *sur) {
    if(sur->data != NULL) {
        free(sur->data);
    }
    sur->data = NULL;
}

void surface_copy(surface *dst, surface *src) {
    surface_create_from_data(dst, src->type, src->w, src->h, src->data);
}

void surface_sub(surface *dst, surface *src, int x, int y, int w, int h) {
    surface_create(dst, src->type, w, h);
    int bytes = (src->type == SURFACE_TYPE_RGBA) ? 4 : 1;

    for(int i = y; i < y+h; i++) {
        for(int j = x; j < x+w; j++) {
            int offset = (i * src->w * bytes) + (j * bytes);
            int local_offset = ((i - y) * w * bytes) + ((j - x) * bytes);
            for(int m = 0; m < bytes; m++) {
                dst->data[local_offset+m] = src->data[offset+m];
            }
        }
    }
}

SDL_Surface* surface_to_sdl(surface *sur, palette *pal) {
    if(sur->type == SURFACE_TYPE_RGBA) {
        return SDL_CreateRGBSurfaceFrom(
            sur->data,
            sur->w,
            sur->h,
            32,
            sur->w * 4,
            0xFF000000,
            0x00FF0000,
            0x0000FF00,
            0x000000FF
        );
    } else {
        char rgba[sur->w * sur->h * 4];
        int src_pos, dst_pos;
        for(src_pos = 0; src_pos < sur->w * sur->h; src_pos++) {
            dst_pos = src_pos * 4;
            int idx = sur->data[src_pos];
            rgba[dst_pos + 0] = pal->data[idx][0];
            rgba[dst_pos + 1] = pal->data[idx][1];
            rgba[dst_pos + 2] = pal->data[idx][2];
            rgba[dst_pos + 3] = (idx > 0) ? 255 : 0;
        }
        return SDL_CreateRGBSurfaceFrom(
            rgba,
            sur->w,
            sur->h,
            32,
            sur->w * 4,
            0xFF000000,
            0x00FF0000,
            0x0000FF00,
            0x000000FF
        );
    }
}