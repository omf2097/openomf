#ifndef AF_H
#define AF_H

#include "resources/af_move.h"

typedef struct af_t {
    unsigned int id;
    float endurance;
    unsigned int health;
    float forward_speed;
    float reverse_speed;
    float jump_speed;
    float fall_speed;
    af_move moves[70];
    char sound_translation_table[30];
} af;

void af_create(af *a, void *src);
af_move *af_get_move(af *a, int id);
void af_free(af *a);

#endif // AF_H
