#ifndef _SPRITE_H
#define _SPRITE_H

#include <inttypes.h>
#include "sprite_image.h"

typedef struct sprite_t {
    int16_t pos_x;
    int16_t pos_y;
    uint8_t index;
    uint8_t missing;
    sprite_image *img;
} sprite;

#endif // _SPRITE_H
