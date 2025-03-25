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
    int id;
    uint32_t last_tick;
    uint32_t max_tick;
    uint8_t last_action;
    hashmap tick_lookup;
    vector game_states;
} wtf;

void rec_controller_free(controller *ctrl) {
    wtf *data = ctrl->data;
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

int get_operand(rec_assertion_operand *op, controller *ctrl) {
    if(op->is_literal) {
        return op->value.literal;
    } else {
        object *obj = game_state_find_object(
            ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, op->value.attr.har_id)));
        har *har = object_get_userdata(obj);
        switch(op->value.attr.attribute) {
            case ATTR_X_POS:
                return obj->pos.x;
            case ATTR_Y_POS:
                return obj->pos.y;
            case ATTR_X_VEL:
                return obj->vel.x;
            case ATTR_Y_VEL:
                return obj->vel.y;
            case ATTR_STATE_ID:
                return har->state;
            case ATTR_ANIMATION_ID:
                return obj->cur_animation->id;
            case ATTR_HEALTH:
                return har->health;
            case ATTR_STAMINA:
                return har->endurance;
            case ATTR_OPPONENT_DISTANCE: {
                object *obj_opp = game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(
                                                                       ctrl->gs, abs(op->value.attr.har_id - 1))));
                return fabsf(obj->pos.x - obj_opp->pos.x);
            }
            case ATTR_DIRECTION:
                return obj->direction;
            default:
                abort();
        }
    }
}

void check_assertion(rec_assertion *ass, controller *ctrl) {
    int16_t operand1 = get_operand(&ass->operand1, ctrl);
    int16_t operand2 = get_operand(&ass->operand2, ctrl);

    log_debug("operand 1 %d operand 2 %d", operand1, operand2);

    switch(ass->op) {
        case OP_EQ:
            if(operand1 != operand2) {
                log_error("%d != %d", operand1, operand2);
                abort();
            }
            break;
        case OP_LT:
            if(operand1 >= operand2) {
                log_error("%d !< %d", operand1, operand2);
                abort();
            }
            break;
        case OP_GT:
            if(operand1 <= operand2) {
                log_error("%d !> %d", operand1, operand2);
                abort();
            }
            break;
        case OP_SET: {
            object *obj = game_state_find_object(
                ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, ass->operand1.value.attr.har_id)));
            har *har = object_get_userdata(obj);

            switch(ass->operand1.value.attr.attribute) {
                case ATTR_X_POS:
                    obj->pos.x = operand2;
                    return;
                case ATTR_Y_POS:
                    obj->pos.y = operand2;
                    return;
                case ATTR_X_VEL:
                    obj->vel.x = operand2;
                    return;
                case ATTR_Y_VEL:
                    obj->vel.y = operand2;
                    return;
                case ATTR_HEALTH:
                    har->health = operand2;
                    return;
                case ATTR_STAMINA:
                    har->endurance = operand2;
                    return;
                default:
                    log_error("unsupported set");
                    abort();
            }
        }
        default:
            abort();
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
                    print_assertion(&ass);
                    check_assertion(&ass, ctrl);
                }
            } else if(move->lookup_id == 2) {
                found_action = true;
                if(move->action == SD_ACT_NONE) {
                    data->last_action = ACT_STOP;
                    controller_cmd(ctrl, ACT_STOP, ev);
                    ctrl->last = ACT_STOP;
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
                        data->last_action = action;
                        controller_cmd(ctrl, action, ev);
                        ctrl->last = action;
                    }
                }
            }
            j++;
        }
        if(!found_action) {
            controller_cmd(ctrl, data->last_action, ev);
        }
    }
    data->last_tick = ticks;
    return 0;
}

int rec_controller_tick(controller *ctrl, uint32_t ticks0, ctrl_event **ev) {
    wtf *data = ctrl->data;
    if(data->id == 0 && ticks0 % 10 == 0) {
        if(scene_is_arena(game_state_get_scene(ctrl->gs)) &&
           game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, 1)))) {
            game_state *gs_bak = vector_append_ptr(&data->game_states);
            game_state_clone(ctrl->gs, gs_bak);
        }
    }
    return 0;
}

void rec_controller_step_back(controller *ctrl) {
    wtf *data = ctrl->data;
    if(data->id != 0 ) {
        return;
    }
    game_state *gs_bak = vector_back(&data->game_states);
    while(vector_size(&data->game_states) > 1 && gs_bak->int_tick >= ctrl->gs->int_tick) {
        game_state_clone_free(gs_bak);
        vector_pop(&data->game_states);
        gs_bak = vector_back(&data->game_states);
    }

    game_state *gs_new = omf_calloc(1, sizeof(game_state));
    game_state_clone(gs_bak, gs_new);
    gs_new->clone = false;
    ctrl->gs->new_state = gs_new;

    log_debug("REWOUND game state from %d to %d", ctrl->gs->int_tick, gs_new->int_tick);

    // fix the game state pointers in the controllers
    for(int i = 0; i < game_state_num_players(gs_new); i++) {
        game_player *gp = game_state_get_player(gs_new, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            c->gs = gs_new;
        }
    }
}

void rec_controller_create(controller *ctrl, int player, sd_rec_file *rec) {
    wtf *data = omf_calloc(1, sizeof(wtf));
    data->last_tick = 0;
    data->last_action = ACT_STOP;
    data->id = player;
    hashmap_create(&data->tick_lookup);
    vector_create(&data->game_states, sizeof(game_state));
    uint32_t last_tick = 0;
    int j = 0;
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
    }
    data->max_tick = rec->moves[rec->move_count - 1].tick;
    log_debug("max tick is %" PRIu32, data->last_tick);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_REC;
    ctrl->poll_fun = &rec_controller_poll;
    ctrl->free_fun = &rec_controller_free;
    ctrl->tick_fun = &rec_controller_tick;
    ctrl->rewind_fun = &rec_controller_step_back;
}
