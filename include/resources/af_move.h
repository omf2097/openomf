#ifndef _AF_MOVE_H
#define _AF_MOVE_H

#include "resources/animation.h"
#include "utils/string.h"

typedef struct af_move_t {
	animation ani;
	str move_string;
	str footer_string;
} af_move;

void af_move_create(af_move *move, void *src);
void af_move_free(af_move *move);

#endif // _AF_MOVE_H
