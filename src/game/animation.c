#include "game/animation.h"
#include "utils/log.h"
#include "video/texture.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

int animation_create(animation *ani, sd_animation *sdani, sd_palette *pal, int overlay, char *soundtable) {
    ani->sdani = sdani;
    array_create(&ani->sprites);
    ani->soundtable = soundtable;
    
    // Load textures
    sd_rgba_image *img = 0;
    for(int i = 0; i < sdani->frame_count; i++) {
        img = sd_sprite_image_decode(sdani->sprites[i]->img, pal, overlay);
        texture *tex = malloc(sizeof(texture));
        texture_create(tex, img->data, img->w, img->h);
        array_insert(&ani->sprites, i, tex);
        sd_rgba_image_delete(img);
    }
    return 0;
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
    ani->sdani = 0;
    ani->soundtable = 0;
}