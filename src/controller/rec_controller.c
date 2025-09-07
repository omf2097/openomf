#include "controller/rec_controller.h"
#include "formats/rec.h"
#include "formats/rec_assertion.h"
#include "game/game_player.h"
#include "game/game_state_type.h"
#include "game/protos/scene.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <inttypes.h>

typedef struct {
    int player_id;
    uint32_t last_tick;
    uint32_t max_tick;
    hashmap tick_lookup;
    vector game_states;
} rec_controller_data;

void rec_controller_free(controller *ctrl) {
    rec_controller_data *data = ctrl->data;
    if(data) {

        iterator it;
        vector_iter_begin(&data->game_states, &it);
        game_state *gs = NULL;
        foreach(it, gs) {
            game_state_clone_free(gs);
        }
        vector_free(&data->game_states);

        hashmap_free(&data->tick_lookup);
        omf_free(data);
    }
}

static int unpack_sd_action(sd_action move_action) {
    assert((move_action & ~SD_ACT_VALID_BITS) == 0);

    if(move_action == SD_ACT_NONE)
        return ACT_STOP;

    int action = 0;
    if(move_action & SD_ACT_UP) {
        action |= ACT_UP;
    }
    if(move_action & SD_ACT_DOWN) {
        action |= ACT_DOWN;
    }
    if(move_action & SD_ACT_LEFT) {
        action |= ACT_LEFT;
    }
    if(move_action & SD_ACT_RIGHT) {
        action |= ACT_RIGHT;
    }
    if(move_action & SD_ACT_PUNCH) {
        action |= ACT_PUNCH;
    }
    if(move_action & SD_ACT_KICK) {
        action |= ACT_KICK;
    }

    return action;
}

int rec_controller_poll(controller *ctrl, ctrl_event **ev) {
    uint32_t ticks = ctrl->gs->tick;
    rec_controller_data *data = ctrl->data;
    sd_rec_move *move;
    unsigned int len;
    if(ticks > data->max_tick) {
        log_debug("closing controller because tick %d is higher than max_tick %d", ticks, data->max_tick);
        controller_close(ctrl, ev);
        return 0;
    }

    uint8_t buf[8];

    bool found_action = false;

    if(data->last_tick != ticks) {
        int j = 0;
        while(hashmap_get_int(&data->tick_lookup, (ticks * 10) + j, (void **)(&move), &len) == 0) {
            if(move->lookup_id == 10) {
                buf[0] = move->raw_action;
                memcpy(buf + 1, move->extra_data, 7);
                rec_assertion ass;
                if(parse_assertion(buf, &ass)) {
                    log_assertion(&ass);
                    if(!game_state_check_assertion_is_met(&ass, ctrl->gs)) {
                        abort();
                    }
                }
            } else if(move->lookup_id == 2) {
                int action = unpack_sd_action(move->action);
                controller_cmd(ctrl, action, ev);
                ctrl->last = action;
                found_action = true;
            }
            j++;
        }
        if(!found_action) {
            controller_cmd(ctrl, ctrl->last, ev);
        }
    }
    if(ticks > data->last_tick) {
        // don't allow this to go backwards if we have an errant out of order event
        data->last_tick = ticks;
    }
    return 0;
}

int rec_controller_dyntick(controller *ctrl, uint32_t ticks, ctrl_event **ev) {
    rec_controller_data *data = ctrl->data;
    if(data->player_id == 0 && ticks % 10 == 0) {
        if(scene_is_arena(game_state_get_scene(ctrl->gs)) &&
           game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, 1)))) {
            game_state *gs_bak = vector_append_ptr(&data->game_states);
            game_state_clone(ctrl->gs, gs_bak);
        }
    }
    return 0;
}

void rec_controller_find_old_last_action(controller *ctrl) {
    rec_controller_data *data = ctrl->data;
    uint32_t ticks = ctrl->gs->tick;

    while(ticks-- != 0) {
        bool found_action = false;
        sd_rec_move *move;
        unsigned int len;
        for(int j = 0; hashmap_get_int(&data->tick_lookup, (ticks * 10) + j, (void **)(&move), &len) == 0; j++) {
            if(move->lookup_id == 2) {
                found_action = true;
                ctrl->last = unpack_sd_action(move->action);
            }
        }
        if(found_action) {
            return;
        }
    }

    // no action found
    ctrl->last = ACT_STOP;
    return;
}

void rec_controller_step_back(controller *ctrl) {
    rec_controller_data *data = ctrl->data;
    if(data->player_id != 0) {
        // first player already handles/handled rewinding, so all we the second
        // player have to do is reset our last_action controller state.
        rec_controller_find_old_last_action(ctrl);
        data->last_tick = ctrl->gs->tick;
        return;
    }
    game_state *gs_bak = vector_back(&data->game_states);
    while(vector_size(&data->game_states) > 1 && gs_bak->tick >= ctrl->gs->tick) {
        game_state_clone_free(gs_bak);
        vector_pop(&data->game_states);
        gs_bak = vector_back(&data->game_states);
    }

    game_state *gs_new = omf_calloc(1, sizeof(game_state));
    game_state_clone(gs_bak, gs_new);
    gs_new->clone = false;
    ctrl->gs->new_state = gs_new;

    data->last_tick = ctrl->gs->tick;

    log_debug("REWOUND game state from %d to %d", ctrl->gs->tick, gs_new->tick);

    // fix the game state pointers in the controllers
    for(int i = 0; i < game_state_num_players(gs_new); i++) {
        game_player *gp = game_state_get_player(gs_new, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            c->gs = gs_new;
        }
    }

    // reset first player controller's last_action state
    rec_controller_find_old_last_action(ctrl);
}

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec) {
    rec_controller_data *data = omf_calloc(1, sizeof(rec_controller_data));
    data->last_tick = 0;
    data->player_id = player;
    hashmap_create(&data->tick_lookup);
    vector_create(&data->game_states, sizeof(game_state));
    uint32_t last_tick = 0;
    int j = 0;
    data->max_tick = 0;
    for(unsigned int i = 0; i < rec->move_count; i++) {
        if(rec->moves[i].player_id == player && (rec->moves[i].lookup_id == 2 || rec->moves[i].lookup_id == 10)) {
            if(last_tick == rec->moves[i].tick) {
                j++;
            } else {
                j = 0;
            }
            hashmap_put_int(&data->tick_lookup, (rec->moves[i].tick * 10) + j, &rec->moves[i], sizeof(sd_rec_move));
            last_tick = rec->moves[i].tick;
        }
        if((rec->moves[i].lookup_id == 2 || rec->moves[i].lookup_id == 10) && rec->moves[i].tick > data->max_tick) {
            data->max_tick = rec->moves[i].tick;
        }
    }
    log_debug("max tick is %" PRIu32, data->last_tick);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_REC;
    ctrl->poll_fun = &rec_controller_poll;
    ctrl->free_fun = &rec_controller_free;
    ctrl->dyntick_fun = &rec_controller_dyntick;
    ctrl->rewind_fun = &rec_controller_step_back;
}
