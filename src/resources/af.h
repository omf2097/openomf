#ifndef AF_H
#define AF_H

#include "resources/af_move.h"
#include "utils/allocator.h"
#include "utils/array.h"

typedef struct af_t {
    unsigned int id;
    float endurance;
    unsigned int health;
    fixedpt forward_speedf;
    fixedpt reverse_speedf;
    fixedpt jump_speedf;
    fixedpt fall_speedf;
    array sprites;
    array moves;
    char sound_translation_table[30];
} af;

void af_create(af *a, void *src);
af_move *af_get_move(const af *a, int id);
void af_free(af *a);

#endif // AF_H
