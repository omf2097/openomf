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

SDL_Texture* surface_to_sdl(surface *sur, SDL_Renderer *renderer, palette *pal, int remap) {
    SDL_Surface *s;
    SDL_Texture *ret;
    if(sur->type == SURFACE_TYPE_RGBA) {
        s = SDL_CreateRGBSurfaceFrom(
            sur->data,
            sur->w,
            sur->h,
            32,
            sur->w * 4,
            0x000000FF,
            0x0000FF00,
            0x00FF0000,
            0xFF000000
        );
        ret = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
    } else {
        char *tmp = malloc(sur->w * sur->h * 4);
        int n = 0;
        uint8_t idx = 0;
        for(int i = 0; i < sur->w * sur->h; i++) {
            n = i * 4;
            if(remap > -1) {
                idx = (uint8_t)pal->remaps[remap][(uint8_t)sur->data[i]];
            } else {
                idx = (uint8_t)sur->data[i];
            }
            *(tmp + n + 0) = pal->data[idx][0];
            *(tmp + n + 1) = pal->data[idx][1];
            *(tmp + n + 2) = pal->data[idx][2];
            *(tmp + n + 3) = (sur->stencil[i] == 1) ? 0xFF : 0;
        }
        s = SDL_CreateRGBSurfaceFrom(
            tmp,
            sur->w,
            sur->h,
            32,
            sur->w*4,
            0x000000FF,
            0x0000FF00,
            0x00FF0000,
            0xFF000000
        );
        ret = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);
        free(tmp);
    }
    return ret;
}