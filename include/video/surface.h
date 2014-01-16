#ifndef _SURFACE_H
#define _SURFACE_H

#include <SDL2/SDL.h>
#include "video/image.h"
#include "video/screen_palette.h"

typedef struct {
    int w;
    int h;
    int type;
    char *data;
    char *stencil;
} surface;

enum {
    SURFACE_TYPE_RGBA,
    SURFACE_TYPE_PALETTE
};

void surface_create(surface *sur, int type, int w, int h);
void surface_create_from_image(surface *sur, image *img);
void surface_create_from_data(surface *sur, int type, int w, int h, const char *src);
void surface_copy(surface *dst, surface *src);
void surface_free(surface *sur);
void surface_sub(surface *dst, surface *src, int x, int y, int w, int h);
SDL_Texture* surface_to_sdl(surface *sur, 
                            SDL_Renderer *renderer, 
                            screen_palette *pal, 
                            char *remap_table,
                            uint8_t pal_offset);

#endif // _SURFACE_H