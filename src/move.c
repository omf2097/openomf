#include "move.h"
#include <stdlib.h>

sd_move* sd_move_create() {
    sd_move *move = (sd_move*)malloc(sizeof(sd_move));
    move->animation.overlay_table=NULL;
    return move;
}

void sd_move_delete(sd_move *move) {
    if (move->animation.overlay_table != NULL) {
        free(move->animation.overlay_table);
    }
    free(move);
}

