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
    free(sur->data);
    free(sur->stencil);
    sur->stencil = NULL;
    sur->data = NULL;
}

int surface_get_type(surface *sur) {
    return sur->type;
}

void surface_clear(surface *sur) {
    if(sur->type == SURFACE_TYPE_RGBA) {
        memset(sur->data, 0, sur->w*sur->h*4);
    } else {
        memset(sur->data, 0, sur->w*sur->h);
    }
}

void surface_copy_ex(surface *dst, surface *src) {
    if(src->type != dst->type) {
        return;
    }
    int size = src->w * src->h * ((src->type == SURFACE_TYPE_PALETTE) ? 1 : 4);
    memcpy(dst->data, src->data, size);
    if(src->stencil != NULL)
        memcpy(dst->stencil, src->stencil, src->w * src->h);
}

// Copies a surface to a new surface
// Note! New surface will be created here; there is no need to pre-create it
void surface_copy(surface *dst, surface *src) {
    surface_create(dst, src->type, src->w, src->h);

    int size = src->w * src->h * ((src->type == SURFACE_TYPE_PALETTE) ? 1 : 4);
    memcpy(dst->data, src->data, size);

    if(src->stencil != NULL && dst->stencil != NULL) {
        memcpy(dst->stencil, src->stencil, src->w * src->h);
    } else {
        dst->stencil = NULL;
    }
}

// Copies a an area of old surface to an entirely new surface
void surface_sub(surface *dst, 
                 surface *src, 
                 int dst_x, int dst_y,
                 int src_x, int src_y, 
                 int w, int h,
                 int method) {

    // Make sure the source and destination are of the same type.
    if(dst->type != src->type) {
        return;
    }

    // Copy!
    int bytes = (src->type == SURFACE_TYPE_RGBA) ? 4 : 1;
    int src_offset,dst_offset;
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            src_offset = (src_x + x + (src_y + y) * src->w) * bytes;
            switch(method) {
                case SUB_METHOD_MIRROR:
                    dst_offset = (dst_x + (w - x - 1) + (dst_y + y) * dst->w) * bytes;
                    break;
                default:
                    dst_offset = (dst_x + x + (dst_y + y) * dst->w) * bytes;
                    break;
            }
            for(int m = 0; m < bytes; m++) {
                dst->data[dst_offset + m] = src->data[src_offset + m];
            }
            if(bytes == 1) {
                dst->stencil[dst_offset] = src->stencil[src_offset];
            }
        }
    }
}

void surface_additive_blit(surface *dst,
                           surface *src,
                           int dst_x, int dst_y,
                           palette *remap_pal,
                           SDL_RendererFlip flip) {

    // Both surfaces must be paletted
    if(dst->type != SURFACE_TYPE_PALETTE || src->type != SURFACE_TYPE_PALETTE) {
        return;
    }

    int src_offset,dst_offset;
    uint8_t src_index, dst_index;
    for(int y = 0; y < src->h; y++) {
        for(int x = 0; x < src->w; x++) {
            src_offset = ((flip & SDL_FLIP_HORIZONTAL) ? src->w - x : x) + 
                         ((flip & SDL_FLIP_VERTICAL) ? src->h - y : y) * src->w;
            dst_offset = dst_x + x + (dst_y + y) * dst->w;
            if(dst->stencil[dst_offset] == 1) {
                if(dst_x + x >= dst->w 
                    || dst_y + y >= dst->h 
                    || dst_x + x < 0 
                    || dst_y + y < 0) continue;
                if(src->data[src_offset] == 0)
                    continue;

                src_index = src->data[src_offset]+3;
                dst_index = dst->data[dst_offset];

                dst->data[dst_offset] = remap_pal->remaps[src_index][dst_index];
            }
        }
    }
}

void surface_alpha_blit(surface *dst,
                        surface *src,
                        int dst_x, int dst_y,
                        SDL_RendererFlip flip) {

    // Both surfaces must be paletted
    if(dst->type != SURFACE_TYPE_PALETTE || src->type != SURFACE_TYPE_PALETTE) {
        return;
    }

    int src_offset,dst_offset;
    for(int y = 0; y < src->h; y++) {
        for(int x = 0; x < src->w; x++) {
            src_offset = ((flip & SDL_FLIP_HORIZONTAL) ? src->w - 1 - x : x) +
                         ((flip & SDL_FLIP_VERTICAL) ? src->h - 1 - y : y) * src->w;
            if(src->stencil[src_offset] == 1) {
                if(dst_x + x >= dst->w 
                    || dst_y + y >= dst->h 
                    || dst_x + x < 0 
                    || dst_y + y < 0) continue;
                dst_offset = dst_x + x + (dst_y + y) * dst->w;
                dst->data[dst_offset] = src->data[src_offset];
                dst->stencil[dst_offset] = 1;
            }
        }
    }
}

// Converts an existing surface to RGBA
void surface_convert_to_rgba(surface *sur, screen_palette *pal, int pal_offset) {
    // Just skip the surface if it already is rgba
    if(sur->type == SURFACE_TYPE_RGBA) {
        return;
    }

    char *pixels = malloc(sur->w * sur->h * 4);
    surface_to_rgba(sur, pixels, pal, NULL, pal_offset);

    // Free old data
    free(sur->data);
    free(sur->stencil);
    sur->data = pixels;
    sur->stencil = NULL;
    sur->type = SURFACE_TYPE_RGBA;
}

// Creates a new RGBA surface
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

// Copies surface to an existing texture.
// Note, texture has to be streaming type
int surface_to_texture(surface *src,
                        SDL_Texture *tex,
                        screen_palette *pal, 
                        char *remap_table,
                        uint8_t pal_offset) {
    void *pixels;
    int pitch;
    if(SDL_LockTexture(tex, NULL, &pixels, &pitch) == 0) {
        surface_to_rgba(src, pixels, pal, remap_table, pal_offset);
        SDL_UnlockTexture(tex);
        return 0;
    }
    PERROR("Failed to lock texture (ptr: %d) for writing: %s", tex, SDL_GetError());
    return 1;
}
