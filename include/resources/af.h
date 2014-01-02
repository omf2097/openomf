#ifndef _AF_H
#define _AF_H

#include "resources/af_move.h"
#include "utils/hashmap.h"

typedef struct af_t {
    unsigned int id;
    unsigned int endurance;
    unsigned int power;
    int forward_speed;
    int reverse_speed;
    int jump_speed;
    int fall_speed;
    hashmap moves;
    char sound_translation_table[30];
} af;

void af_create(af *a, void *src);
af_move* af_get_move(af *a, int id);
void af_free(af *a);

#endif // _AF_H
