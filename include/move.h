#ifndef MOVE_H
#define MOVE_H

#include "animation.h"

typedef struct sd_move_t {
    sd_animation animation;
    char unknown[21];
    char move_string[21];
    char *footer_string;
} sd_move;

sd_move* sd_move_create();
void sd_move_delete(sd_move *move);

#endif // _ANIMATION_H
