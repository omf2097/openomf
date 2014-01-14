#include <shadowdive/shadowdive.h>
#include <stdlib.h>
#include <string.h>
#include "resources/sprite.h"

void sprite_create_custom(sprite *sp, vec2i pos, surface *data) {
    sp->id = -1;
    sp->pos = pos;
    sp->data = data;
    sp->stencil = NULL; // TODO: Does this need to be generated ?
}

void sprite_create(sprite *sp, void *src, int id) {
    sd_sprite *sdsprite = (sd_sprite*)src;
    sp->id = id;
    sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
    sp->data = malloc(sizeof(surface));

    // Load data
    sd_vga_image *raw = sd_sprite_vga_decode(sdsprite->img);
    sp->stencil = malloc(raw->w * raw->h);
    memcpy(sp->stencil, raw->stencil, raw->w * raw->h);
    surface_create_from_data(sp->data, SURFACE_TYPE_PALETTE, raw->w, raw->h, raw->data);
    sd_vga_image_delete(raw);
}

void sprite_free(sprite *sp) {
    surface_free(sp->data);
    free(sp->data);
    free(sp->stencil);
    sp->data = NULL;
}

vec2i sprite_get_size(sprite *sp) {
    if(sp->data != NULL) {
        return vec2i_create(sp->data->w, sp->data->h);
    }
    return vec2i_create(0,0);
}

sprite* sprite_copy(sprite *src) {
    sprite *new = malloc(sizeof(sprite));
    new->pos = src->pos;
    new->data = malloc(sizeof(surface));
    surface_copy(new->data, src->data);
    return new;
}