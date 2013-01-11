#include "game/animation.h"
#include "utils/log.h"
#include "video/texture.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void animation_create(animation *ani, sd_bk_anim *bka, sd_palette *pal, int overlay) {
    ani->bka = bka;
    array_create(&ani->sprites);
    
    sd_rgba_image *img = 0;
    for(int i = 0; i < bka->animation->frame_count; i++) {
        DEBUG("* Start sprite %u", i);
        img = sd_sprite_image_decode(bka->animation->sprites[i]->img, pal, overlay);
        texture *tex = malloc(sizeof(texture));
        texture_create(tex, img->data, img->w, img->h);
        array_insert(&ani->sprites, i, tex);
        sd_rgba_image_delete(img);
        DEBUG("* End sprite %i", i);
    }
}

void animation_free(animation *ani) {
    texture *ptr;
    array_iterator it;
    array_iter(&ani->sprites, &it);
    while((ptr = array_next(&it)) != 0) {
        texture_free(ptr);
        free(ptr);
    }
    array_free(&ani->sprites);
    ani->bka = 0;
}