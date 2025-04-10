#ifndef AF_MOVE_H
#define AF_MOVE_H

#include "resources/animation.h"
#include "utils/array.h"
#include "utils/str.h"

#define JUMP_COORD_ADJUSTMENT 60

typedef struct af_move_t {
    int id;
    animation ani;
    uint8_t pos_constraints;
    uint8_t next_move;
    uint8_t successor_id;
    uint8_t category;
    uint16_t points;
    uint8_t block_damage;
    uint8_t block_stun;
    uint8_t throw_duration;
    uint8_t extra_string_selector;
    float damage;
    float raw_damage; // used to store the raw damage, unmodified by pilot stats
    float stun;
    str move_string;
    str footer_string;
#ifdef DEBUGMODE
    char unknown[21];
#endif
} af_move;

void af_move_create(af_move *move, array *sprites, void *src, int id);
void af_move_free(af_move *move);

#endif // AF_MOVE_H
