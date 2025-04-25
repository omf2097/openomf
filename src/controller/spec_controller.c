#include "controller/spec_controller.h"
#include "game/game_player.h"
#include "game/game_state_type.h"
#include "game/protos/scene.h"
#include "game/scenes/arena.h"
#include "game/scenes/vs.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct {
    uint32_t ticks;
    uint8_t actions[2][10];
} spec_controller_event;

typedef struct {
    int player_id;
    int nscene;
    bool started;
    ENetHost *host;
    ENetPeer *peer;
    uint32_t last_tick;
    uint32_t max_tick;
    uint32_t start_tick;
    hashmap *tick_lookup;
} spec_controller_data;

void spec_controller_free(controller *ctrl) {
    spec_controller_data *data = ctrl->data;
    if(data) {
        if(data->player_id == 0) {
            hashmap_free(data->tick_lookup);
        }
        omf_free(data);
    }
}

int spec_controller_tick(controller *ctrl, uint32_t ticks0, ctrl_event **ev) {
    uint32_t ticks = ctrl->gs->tick;
    ENetEvent event;
    spec_controller_data *data = ctrl->data;
    ENetHost *host = data->host;
    // ENetPeer *peer = data->peer;
    serial ser;

    // only poll on one controller, use the shared hashmap to send events to the other controller
    while(data->player_id == 0 && enet_host_service(host, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
                switch(serial_read_int8(&ser)) {
                    case 0: {
                        // init packet, describes the pilots and arena, we can use this to start the arena
                        game_player *p1 = game_state_get_player(ctrl->gs, 0);
                        game_player *p2 = game_state_get_player(ctrl->gs, 1);

                        // force the speed to 3
                        game_state_set_speed(ctrl->gs, 10);

                        p1->pilot->har_id = serial_read_int8(&ser);
                        p1->pilot->pilot_id = serial_read_int8(&ser);
                        p1->pilot->power = serial_read_int8(&ser);
                        p1->pilot->agility = serial_read_int8(&ser);
                        p1->pilot->endurance = serial_read_int8(&ser);
                        sd_pilot_set_player_color(p1->pilot, PRIMARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p1->pilot, SECONDARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p1->pilot, TERTIARY, serial_read_int8(&ser));
                        uint8_t namelen = serial_read_int8(&ser);
                        serial_read(&ser, p1->pilot->name, namelen);
                        p1->pilot->name[namelen] = '\0';

                        p2->pilot->har_id = serial_read_int8(&ser);
                        p2->pilot->pilot_id = serial_read_int8(&ser);
                        p2->pilot->power = serial_read_int8(&ser);
                        p2->pilot->agility = serial_read_int8(&ser);
                        p2->pilot->endurance = serial_read_int8(&ser);
                        sd_pilot_set_player_color(p2->pilot, PRIMARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p2->pilot, SECONDARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p2->pilot, TERTIARY, serial_read_int8(&ser));
                        namelen = serial_read_int8(&ser);
                        serial_read(&ser, p2->pilot->name, namelen);
                        p2->pilot->name[namelen] = '\0';

                        data->nscene = SCENE_ARENA0 + serial_read_int8(&ser);

                        ctrl->gs->arena = data->nscene - SCENE_ARENA0;

                        // jump into the arena scene
                        ctrl->gs->this_id = SCENE_VS;
                        ctrl->gs->next_id = SCENE_VS;

                        if(scene_create(ctrl->gs->sc, ctrl->gs, SCENE_VS)) {
                            log_error("Error while loading scene %d.", SCENE_VS);
                        }

                        if(vs_create(ctrl->gs->sc)) {
                            log_error("Error while creating arena");
                        }

                    } break;
                    case 1: {
                        uint8_t action;
                        for(size_t i = ser.rpos; i < event.packet->dataLength;) {
                            spec_controller_event event;
                            event.ticks = serial_read_uint32(&ser);

                            for(int j = 0; j < 2; j++) {
                                int k = 0;
                                do {
                                    action = serial_read_int8(&ser);
                                    log_debug("tick %d read action %d at %d for player %d as position %d", event.ticks,
                                              action, k, j, i);
                                    event.actions[j][k] = action;
                                    k++;
                                } while(action);
                                i += k;
                            }
                            i += 4;
                            hashmap_put_int(data->tick_lookup, event.ticks, &event, sizeof(spec_controller_event));

                            if(event.ticks > 100 && !data->started) {
                                // insert the starting tick into the hashmap so we can offset all events from that
                                hashmap_put_int(data->tick_lookup, 0, &ctrl->gs->tick, sizeof(ticks));
                                log_info("spectator start tick was %d", ticks);

                                // jump into the arena scene
                                // ctrl->gs->this_id = data->nscene;
                                ctrl->gs->next_id = data->nscene;

                                if(scene_create(ctrl->gs->sc, ctrl->gs, data->nscene)) {
                                    log_error("Error while loading scene %d.", data->nscene);
                                }

                                if(arena_create(ctrl->gs->sc)) {
                                    log_error("Error while creating arena");
                                }
                                data->started = true;
                            }
                        }
                    } break;
                    default: {
                    }
                }
            default: {
            }
        }
    }
    return 0;
}

int spec_controller_poll(controller *ctrl, ctrl_event **ev) {
    uint32_t ticks = ctrl->gs->tick;
    spec_controller_data *data = ctrl->data;
    spec_controller_event *move;
    unsigned int len;
    if(data->max_tick && ticks > data->max_tick) {
        log_debug("closing controller because tick %d is higher than max_tick %d", ticks, data->max_tick);
        controller_close(ctrl, ev);
        return 0;
    }

    int *start_tick;

    if(hashmap_get_int(data->tick_lookup, 0, (void **)(&start_tick), NULL) != 0) {
        return 0;
    }

    bool found_action = false;

    if(data->last_tick != ticks) {
        if(hashmap_get_int(data->tick_lookup, ticks, (void **)(&move), &len) == 0) {
            int i = 0;
            uint8_t action;
            while((action = move->actions[data->player_id][i])) {
                controller_cmd(ctrl, action, ev);
                ctrl->last = action;
                found_action = true;
                i++;
            }
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

void spec_controller_find_old_last_action(controller *ctrl) {
    spec_controller_data *data = ctrl->data;
    uint32_t ticks = ctrl->gs->tick;

    while(ticks-- != 0) {
        bool found_action = false;
        spec_controller_event *move;
        unsigned int len;
        if(hashmap_get_int(data->tick_lookup, ticks, (void **)(&move), &len) == 0) {
            int i = 0;
            uint8_t action;
            while((action = move->actions[data->player_id][i])) {
                found_action = true;
                ctrl->last = action;
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

ENetPeer *spec_controller_get_lobby_connection(controller *ctrl) {
    spec_controller_data *data = ctrl->data;
    return data->peer;
}

ENetHost *spec_controller_get_host(controller *ctrl) {
    spec_controller_data *data = ctrl->data;
    return data->host;
}

void spec_controller_create(controller *ctrl, int player, ENetHost *host, ENetPeer *lobby, hashmap *events) {
    spec_controller_data *data = omf_calloc(1, sizeof(spec_controller_data));
    data->last_tick = 0;
    data->player_id = player;
    data->tick_lookup = events;
    data->host = host;
    data->peer = lobby;
    data->max_tick = 0;
    data->started = false;
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_SPECTATOR;
    ctrl->poll_fun = &spec_controller_poll;
    ctrl->tick_fun = &spec_controller_tick;
    ctrl->free_fun = &spec_controller_free;
}
