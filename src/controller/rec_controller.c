#include "controller/rec_controller.h"
#include "utils/log.h"

typedef struct wtf_t {
    int id;
    unsigned int ticks;
    int last_action;
    hashmap tick_lookup;
} wtf;

int rec_controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    wtf *data = ctrl->data;
    sd_rec_move *move;
    unsigned int len;
    if (hashmap_iget(&data->tick_lookup, data->ticks, (void**)(&move), &len) == 0) {
        DEBUG("tick %d has action %s for player %d", data->ticks, move->raw_action, move->player_id);
    }
    data->ticks++;
    return 0;
}

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec) {
    wtf *data = malloc(sizeof(wtf));
    data->last_action = ACT_STOP;
    data->ticks = 0;
    hashmap_create(&data->tick_lookup, rec->move_count);
    DEBUG("recording has %d moves", rec->move_count);
    for(unsigned int i = 0; i < rec->move_count; i++) {
        if (rec->moves[i].player_id == player && rec->moves[i].extra == 2) {
            hashmap_iput(&data->tick_lookup, i, &rec->moves[i], sizeof(sd_rec_move));
        }
    }
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->tick_fun = &rec_controller_tick;
}
