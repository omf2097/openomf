#include "controller/spec_controller.h"
#include "controller/net_controller.h"
#include "game/common_defines.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct {
    uint32_t ticks;
    uint8_t actions[2][MAX_EVENTS_PER_TICK];
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
                        match_settings ms;
                        // init packet, describes the pilots and arena, we can use this to start the arena
                        game_player *p1 = game_state_get_player(ctrl->gs, 0);
                        game_player *p2 = game_state_get_player(ctrl->gs, 1);

                        // force the speed to 3
                        game_state_set_speed(ctrl->gs, 10);

                        // decode the match settings from the lobby and apply them
                        game_state_decode_match_settings(&ser, &ms);
                        game_state_copy_match_settings(ctrl->gs, &ms);

                        p1->pilot->har_id = serial_read_int8(&ser);
                        p1->pilot->pilot_id = serial_read_int8(&ser);
                        p1->pilot->power = serial_read_int8(&ser);
                        p1->pilot->agility = serial_read_int8(&ser);
                        p1->pilot->endurance = serial_read_int8(&ser);
                        sd_pilot_set_player_color(p1->pilot, PRIMARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p1->pilot, SECONDARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p1->pilot, TERTIARY, serial_read_int8(&ser));
                        serial_read_str(&ser, &p1->pilot->name);

                        p2->pilot->har_id = serial_read_int8(&ser);
                        p2->pilot->pilot_id = serial_read_int8(&ser);
                        p2->pilot->power = serial_read_int8(&ser);
                        p2->pilot->agility = serial_read_int8(&ser);
                        p2->pilot->endurance = serial_read_int8(&ser);
                        sd_pilot_set_player_color(p2->pilot, PRIMARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p2->pilot, SECONDARY, serial_read_int8(&ser));
                        sd_pilot_set_player_color(p2->pilot, TERTIARY, serial_read_int8(&ser));
                        serial_read_str(&ser, &p2->pilot->name);

                        uint32_t seed = serial_read_uint32(&ser);
                        log_debug("spectator random seed set to %u", seed);
                        random_seed(&ctrl->gs->rand, seed);

                        data->nscene = SCENE_ARENA0 + serial_read_int8(&ser);

                        ctrl->gs->arena = data->nscene - SCENE_ARENA0;

                        // jump into the VS scene, keeping the lobby scene and its
                        // network connection alive
                        if(game_state_swap_scene(ctrl->gs, SCENE_VS)) {
                            log_error("Error while loading scene %d.", SCENE_VS);
                        }
                    } break;
                    case 1: {
                        while(ser.rpos < ser.wpos) {
                            spec_controller_event spec_event = {0};
                            spec_event.ticks = serial_read_uint32(&ser);
                            serial_read_bytes(&ser, spec_event.actions[0], MAX_EVENTS_PER_TICK);
                            serial_read_bytes(&ser, spec_event.actions[1], MAX_EVENTS_PER_TICK);
                            hashmap_put_int(data->tick_lookup, spec_event.ticks, &spec_event,
                                            sizeof(spec_controller_event));

                            if(spec_event.ticks > 100 && !data->started) {
                                // insert the starting tick into the hashmap so we can offset all events from that
                                hashmap_put_int(data->tick_lookup, 0, &ctrl->gs->tick, sizeof(ticks));
                                log_info("spectator start tick was %u", ticks);

                                // jump into the arena scene, keeping the old scene alive
                                if(game_state_swap_scene(ctrl->gs, data->nscene)) {
                                    log_error("Error while loading scene %d.", data->nscene);
                                }
                                data->started = true;
                            }
                        }
                    } break;
                    default: {
                    }
                }
                serial_free(&ser);
                enet_packet_destroy(event.packet);
                break;
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
        log_debug("closing controller because tick %u is higher than max_tick %u", ticks, data->max_tick);
        controller_close(ctrl, ev);
        return 0;
    }

    int *start_tick;

    if(hashmap_get_int(data->tick_lookup, 0, (void **)(&start_tick), NULL) != 0) {
        return 0;
    }

    if(data->last_tick != ticks && ticks > 0) {
        bool found_action = false;
        if(hashmap_get_int(data->tick_lookup, ticks, (void **)(&move), &len) == 0) {
            int i = 0;
            uint8_t action;
            while(i < MAX_EVENTS_PER_TICK && (action = move->actions[data->player_id][i])) {
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
