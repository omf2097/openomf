#include "controller/rec_controller.h"
#include "formats/rec.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <inttypes.h>

typedef struct {
    int id;
    uint32_t last_tick;
    uint32_t max_tick;
    hashmap tick_lookup;
} wtf;

void rec_controller_free(controller *ctrl) {
    wtf *data = ctrl->data;
    if(data) {
        hashmap_free(&data->tick_lookup);
        omf_free(data);
    }
}

int rec_controller_poll(controller *ctrl, ctrl_event **ev) {
    uint32_t ticks = ctrl->gs->int_tick;
    wtf *data = ctrl->data;
    sd_rec_move *move;
    unsigned int len;
    if(ticks > data->max_tick) {
        log_debug("closing controller");
        controller_close(ctrl, ev);
        return 0;
    }

    if(data->last_tick != ticks) {
        if(hashmap_get_int(&data->tick_lookup, ticks, (void **)(&move), &len) == 0) {
            if(move->action == SD_ACT_NONE) {
                controller_cmd(ctrl, ACT_STOP, ev);
            } else {
                int action = 0;
                if(move->action & SD_ACT_UP) {
                    action |= ACT_UP;
                }

                if(move->action & SD_ACT_DOWN) {
                    action |= ACT_DOWN;
                }

                if(move->action & SD_ACT_LEFT) {
                    action |= ACT_LEFT;
                }

                if(move->action & SD_ACT_RIGHT) {
                    action |= ACT_RIGHT;
                }
                if(move->action & SD_ACT_PUNCH) {
                    action |= ACT_PUNCH;
                }
                if(move->action & SD_ACT_KICK) {
                    action |= ACT_KICK;
                }

                if(action != 0) {
                    controller_cmd(ctrl, action, ev);
                }
            }
        }
    }
    data->last_tick = ticks;
    return 0;
}

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec) {
    wtf *data = omf_calloc(1, sizeof(wtf));
    data->last_tick = 0;
    hashmap_create(&data->tick_lookup);
    for(unsigned int i = 0; i < rec->move_count; i++) {
        if(rec->moves[i].player_id == player && rec->moves[i].lookup_id == 2) {
            hashmap_put_int(&data->tick_lookup, rec->moves[i].tick, &rec->moves[i], sizeof(sd_rec_move));
        }
    }
    data->max_tick = rec->moves[rec->move_count - 1].tick;
    log_debug("max tick is %" PRIu32, data->last_tick);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_REC;
    ctrl->poll_fun = &rec_controller_poll;
    ctrl->free_fun = &rec_controller_free;
}
