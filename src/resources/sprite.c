#include "formats/sprite.h"
#include "resources/sprite.h"
#include "utils/allocator.h"
#include <stdlib.h>
#include <string.h>

void sprite_create_custom(sprite *sp, vec2i pos, surface *data) {
    sp->id = -1;
    sp->pos = pos;
    sp->data = data;
}

void sprite_create(sprite *sp, void *src, int id) {
    sd_sprite *sdsprite = (sd_sprite *)src;
    sp->id = id;
    sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
    sp->data = omf_calloc(1, sizeof(surface));

    // Load data
    sd_vga_image raw;
    sd_sprite_vga_decode(&raw, sdsprite);
    surface_create_from_data(sp->data, SURFACE_TYPE_PALETTE, raw.w, raw.h, raw.data);
    memcpy(sp->data->stencil, raw.stencil, raw.w * raw.h);
    sd_vga_image_free(&raw);
}

int sprite_clone(sprite *src, sprite *dst) {
    memcpy(dst, src, sizeof(sprite));
    dst->data = omf_calloc(1, sizeof(surface));
    surface_copy_ex(dst->data, src->data);
    return 0;
}

void sprite_free(sprite *sp) {
    surface_free(sp->data);
    omf_free(sp->data);
}

vec2i sprite_get_size(sprite *sp) {
    if(sp->data != NULL) {
        return vec2i_create(sp->data->w, sp->data->h);
    }
    return vec2i_create(0, 0);
}

sprite *sprite_copy(sprite *src) {
    if(src == NULL)
        return NULL;

    sprite *new = omf_calloc(1, sizeof(sprite));
    new->pos = src->pos;
    new->id = src->id;

    // Copy surface
    new->data = omf_calloc(1, sizeof(surface));
    surface_copy(new->data, src->data);
    return new;
}
