#include <stdlib.h>
#include <string.h>
#include <utils/log.h>
#include "video/surface.h"

void surface_create(surface *sur, int type, int w, int h) {
    if(type == SURFACE_TYPE_RGBA) {
        sur->data = malloc(w*h*4);
        sur->stencil = NULL;
    } else {
        sur->data = malloc(w*h);
        sur->stencil = malloc(w*h);
    }
    sur->w = w;
    sur->h = h;
    sur->type = type;
}

void surface_create_from_data(surface *sur, int type, int w, int h, const char *src) {
    surface_create(sur, type, w, h);
    int size = w * h * ((type == SURFACE_TYPE_PALETTE) ? 1 : 4);
    memcpy(sur->data, src, size);
    if(type == SURFACE_TYPE_PALETTE) {
        memset(sur->stencil, 1, w * h);
    }
}

void surface_create_from_image(surface *sur, image *img) {
    surface_create_from_data(sur, SURFACE_TYPE_RGBA, img->w, img->h, img->data);
}

void surface_free(surface *sur) {
    if(sur->data != NULL) {
        free(sur->data);
        sur->data = NULL;
    }
    if(sur->stencil != NULL) {
        free(sur->stencil);
        sur->stencil = NULL;
    }
}

void surface_copy(surface *dst, surface *src) {
    surface_create(dst, src->type, src->w, src->h);

    int size = src->w * src->h * ((src->type == SURFACE_TYPE_PALETTE) ? 1 : 4);
    memcpy(dst->data, src->data, size);

    if(src->stencil != NULL) {
        memcpy(dst->stencil, src->stencil, src->w * src->h);
    } else {
        dst->stencil = NULL;
    }
}

void surface_sub(surface *dst, surface *src, int x, int y, int w, int h) {
    surface_create(dst, src->type, w, h);
    int bytes = (src->type == SURFACE_TYPE_RGBA) ? 4 : 1;

    for(int i = y; i < y + h; i++) {
        for(int j = x; j < x + w; j++) {
            int offset = (i * src->w * bytes) + (j * bytes);
            int local_offset = ((i - y) * w * bytes) + ((j - x) * bytes);
            for(int m = 0; m < bytes; m++) {
                dst->data[local_offset+m] = src->data[offset+m];
            }
            if(bytes == 1) {
                dst->stencil[local_offset] = src->stencil[offset];
            }
        }
    }
}

void surface_to_rgba(surface *sur,
                     char *dst,
                     screen_palette *pal, 
                     char *remap_table,
                     uint8_t pal_offset) {

    if(sur->type == SURFACE_TYPE_RGBA) {
        memcpy(dst, sur->data, sur->w * sur->h * 4);
    } else {
        int n = 0;
        uint8_t idx = 0;
        for(int i = 0; i < sur->w * sur->h; i++) {
            n = i * 4;
            if(remap_table != NULL) {
                idx = (uint8_t)remap_table[(uint8_t)sur->data[i]];
            } else {
                idx = (uint8_t)sur->data[i];
            }
            // TODO: This is kind of a hack. Since the pal_offset
            // is only ever used for player 2 har, we can safely
            // make some assumptions. therefore, only apply offset,
            // if the color we are handling is between 0 and 48 (har colors).
            if(idx < 48) {
                idx += pal_offset;
            }
            *(dst + n + 0) = pal->data[idx][0];
            *(dst + n + 1) = pal->data[idx][1];
            *(dst + n + 2) = pal->data[idx][2];
            *(dst + n + 3) = (sur->stencil[i] == 1) ? 0xFF : 0;
        }
    }
}