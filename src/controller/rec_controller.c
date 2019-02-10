#include "controller/rec_controller.h"
#include "formats/rec.h"
#include "formats/error.h"
#include "utils/log.h"

typedef struct wtf_t {
    int id;
    int last_tick;
    int last_action;
    int max_tick;
    hashmap tick_lookup;
} wtf;

int rec_controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    wtf *data = ctrl->data;
    sd_rec_move *move;
    unsigned int len;
    if (ticks > data->max_tick) {
        DEBUG("closing controller");
        controller_close(ctrl, ev);
        return 0;
    }

    if (data->last_tick != ticks) {
        if (hashmap_iget(&data->tick_lookup, ticks, (void**)(&move), &len) == 0) {
            if (move->action == SD_ACT_NONE) {
                controller_cmd(ctrl, ACT_STOP, ev);
                data->last_action = ACT_STOP;
            } else {
                if (move->action & SD_ACT_PUNCH) {
                    controller_cmd(ctrl, ACT_PUNCH, ev);
                } else if (move->action & SD_ACT_KICK) {
                    controller_cmd(ctrl, ACT_KICK, ev);
                }

                int action = 0;
                if (move->action & SD_ACT_UP) {
                    action |= ACT_UP;
                }

                if (move->action & SD_ACT_DOWN) {
                    action |= ACT_DOWN;
                }

                if (move->action & SD_ACT_LEFT) {
                    action |= ACT_LEFT;
                }

                if (move->action & SD_ACT_RIGHT) {
                    action |= ACT_RIGHT;
                }
                if (action != 0) {
                    controller_cmd(ctrl, action, ev);
                    data->last_action = action;
                } else {
                    data->last_action = ACT_STOP;
                }
            }
        } else {
            controller_cmd(ctrl, data->last_action, ev);
        }
    }
    data->last_tick = ticks;
    return 0;
}

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec) {
    wtf *data = malloc(sizeof(wtf));
    data->last_action = ACT_STOP;
    data->last_tick = 0;
    hashmap_create(&data->tick_lookup, 9);
    for(unsigned int i = 0; i < rec->move_count; i++) {
        if (rec->moves[i].player_id == player && rec->moves[i].lookup_id == 2) {
            hashmap_iput(&data->tick_lookup, rec->moves[i].tick, &rec->moves[i], sizeof(sd_rec_move));
        }
    }
    data->max_tick = rec->moves[rec->move_count-1].tick;
    DEBUG("max tick is %d", data->last_tick);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_REC;
    ctrl->dyntick_fun = &rec_controller_tick;
}
