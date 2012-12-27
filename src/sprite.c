#include "sprite.h"
#include <stdlib.h>

sd_sprite* sd_sprite_create() {
    sd_sprite *sprite = (sd_sprite*)malloc(sizeof(sd_sprite));
    return sprite;
}

void sd_sprite_delete(sd_sprite *sprite) {
    free(sprite);
}
