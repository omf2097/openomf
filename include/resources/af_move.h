#ifndef _AF_MOVE_H
#define _AF_MOVE_H

#include "resources/animation.h"
#include "utils/str.h"

typedef struct af_move_t {
    int id;
    animation ani;
    uint8_t pos_constraints;
    uint8_t next_move;
    uint8_t successor_id;
    uint8_t category;
    uint16_t points;
    uint8_t scrap_amount;
    float damage;
    str move_string;
    str footer_string;
#ifdef DEBUGMODE
    char unknown[21];
#endif
} af_move;

void af_move_create(af_move *move, void *src, int id);
void af_move_free(af_move *move);

#endif // _AF_MOVE_H
