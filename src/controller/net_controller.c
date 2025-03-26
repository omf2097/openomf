#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include "controller/net_controller.h"
#include "game/game_state_type.h"
#include "game/protos/scene.h"
#include "game/scenes/arena.h"
#include "game/utils/serial.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"
#include "utils/miscmath.h"

typedef struct {
    ENetHost *host;
    ENetPeer *peer;
    ENetPeer *lobby;
    int id;
    uint32_t last_hb;
    uint32_t outstanding_hb;
    int disconnected;
    int rttbuf[100];
    int rttpos;
    int rttfilled;
    int tick_offset;
    int frame_advantage;
    bool synchronized;
    int guesses;
    uint32_t peer_proposal;
    uint32_t local_proposal;
    bool confirmed;
    uint32_t last_tick;
    uint32_t last_sent;
    list transcript;
    uint32_t last_received_tick;
    uint32_t last_acked_tick;
    int last_har_state;
    uint32_t last_traced_tick;
    uint32_t peer_last_hash;
    uint32_t peer_last_hash_tick;
    uint32_t last_hash;
    uint32_t last_hash_tick;
    uint8_t last_action;
    int8_t last_direction;
    SDL_RWops *trace_file;
    game_state *gs_bak;
    int winner;
} wtf;

typedef struct {
    uint32_t tick;
    uint8_t events[2][11];
    int8_t direction[2];
} tick_events;

// simple standard deviation calculation
float stddev(float average, int data[], int n) {
    float variance = 0.0f;
    for(int i = 0; i < n; i++) {
        variance = (data[i] - average) * (data[i] - average);
    }
    return sqrtf(variance / n);
}

// calculate average round trip time, ignoring outliers outside 2 standard deviations
int avg_rtt(int data[], int n) {
    float average = 0.0f;
    float sum = 0.0f;
    float result = 0.0f;
    int j = 0;
    for(int i = 0; i < n; i++) {
        sum += data[i];
    }
    average = sum / (n * 1.0);

    float sd = stddev(average, data, n);
    // log_debug("sum %f, stddev %f, average %f", sum, sd, average);
    for(int i = 0; i < n; i++) {
        if(fabsf(data[i] - average) <= sd * 2) {
            result += data[i];
            j++;
        }
    }
    if(j != 0) {
        return truncf(result / j);
    }
    return truncf(average);
}

// insert an event into the event trace
void insert_event(wtf *data, uint32_t tick, uint16_t action, int id, int direction) {

    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    tick_events *nev = NULL;
    tick_events *last_ev = NULL;
    tick_events event;
    event.tick = tick;
    memset(event.events[id], 0, 11);
    memset(event.events[abs(id - 1)], 0, 11);
    event.events[id][0] = action;
    event.direction[id] = direction;
    int i = 0;

    foreach(it, ev) {
        if(id == data->id && ev->events[id][0]) {
            // we had events this tick, so get the last action and direction
            last_ev = ev;
            data->last_direction = last_ev->direction[id];
            int j = 0;
            while(last_ev->events[id][j] != 0) {
                data->last_action = last_ev->events[id][j];
                j++;
            }
        }
        if(i == 0 && ev->tick > tick) {
            list_prepend(transcript, &event, sizeof(tick_events));
            goto done;
        } else if(ev->tick == tick) {
            if(id == data->id && action == data->last_action && data->last_direction &&
               data->last_direction == direction) {
                // dedup;
                return;
            }

            for(int j = 0; j < 11; j++) {
                if(ev->events[id][j] == 0) {
                    ev->events[id][j] = action;
                    ev->direction[id] = direction;
                    break;
                }
            }
            goto done;
        }
        nev = iter_peek(&it);
        if(ev->tick < tick && nev && nev->tick > tick) {
            if(id == data->id && action == data->last_action && data->last_direction &&
               data->last_direction == direction) {
                // dedup
                return;
            }
            list_iter_append(&it, &event, sizeof(tick_events));
            goto done;
        }
        i++;
    }
    // either the list is empty, or this tick is later than anything else
    if(id == data->id && action == data->last_action && data->last_direction && data->last_direction == direction) {
        // dedup;
        return;
    }
    list_append(transcript, &event, sizeof(tick_events));
done:
    if(id == data->id) {
        data->last_action = action;
        data->last_direction = direction;
    }
}

// check if we have any events for this tick
bool has_event(wtf *data, uint32_t tick) {

    iterator it;
    list_iter_begin(&data->transcript, &it);
    tick_events *ev = NULL;
    foreach(it, ev) {
        if(ev->tick == tick - data->local_proposal && ev->events[data->id][0]) {
            return true;
        }
    }
    return false;
}

void event_names(char *buf, uint8_t *actions) {

    for(int i = 0; i < 11; i++) {
        uint8_t action = actions[i];
        if(action == ACT_STOP) {
            buf[0] = '5';
            buf[1] = '\0';
            return;
        }

        if(action & ACT_STOP) {
            // should not appear with others
            assert(false);
        }

        if(action & ACT_DOWN && action & ACT_LEFT) {
            *buf++ = '1';
        } else if(action & ACT_DOWN && action & ACT_RIGHT) {
            *buf++ = '3';
        } else if(action & ACT_DOWN) {
            *buf++ = '2';
        } else if(action & ACT_UP && action & ACT_LEFT) {
            *buf++ = '7';
        } else if(action & ACT_UP && action & ACT_RIGHT) {
            *buf++ = '9';
        } else if(action & ACT_UP) {
            *buf++ = '8';
        } else if(action & ACT_LEFT) {
            *buf++ = '4';
        } else if(action & ACT_RIGHT) {
            *buf++ = '4';
        } else if(action & ACT_DOWN || action & ACT_LEFT || action & ACT_RIGHT || action & ACT_LEFT) {
            // invalid combination of arrow keys
            assert(false);
        }

        if(action & ACT_KICK) {
            *buf++ = 'k';
        }

        if(action & ACT_PUNCH) {
            *buf++ = 'p';
        }
    }

    *buf++ = '\0';
}

void print_transcript(list *transcript) {
    iterator it;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    foreach(it, ev) {
        log_debug("tick %d has events %d -- %d", ev->tick, ev->events[0], ev->events[1]);
    }
}

// send any events we've made that are older than the last acked event from the peer
void send_events(wtf *data) {
    serial ser;
    ENetPacket *packet;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    list *transcript = &data->transcript;
    iterator it;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    serial_create(&ser);
    serial_write_int8(&ser, EVENT_TYPE_ACTION);
    serial_write_uint32(&ser, data->last_received_tick);
    serial_write_uint32(&ser, data->last_hash_tick);
    serial_write_uint32(&ser, data->last_hash);
    serial_write_uint32(&ser, data->last_tick - data->local_proposal);
    serial_write_int8(&ser, data->frame_advantage);

    int events = 0;

    int last_sent = 0;

    foreach(it, ev) {
        if(ev->events[data->id][0] != 0 && ev->tick > data->last_acked_tick &&
           ev->tick < data->last_tick - data->local_proposal) {
            serial_write_uint32(&ser, ev->tick);
            int i = 0;
            while(ev->events[data->id][i]) {
                serial_write_int8(&ser, ev->events[data->id][i]);
                i++;
            }
            serial_write_int8(&ser, 0);
            last_sent = ev->tick;
            events++;
        }
    }

    if(!events) {
        // nothing to send, so send a blank event
        // serial_write_int16(&ser, 0);
        // serial_write_int32(&ser, data->last_tick - data->local_proposal - 1);
        // last_sent = data->last_tick - data->local_proposal - 1;
    }

    data->last_sent = max2(data->last_sent, last_sent);

    packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, 2, packet);
    if(data->lobby && peer != data->lobby) {
        // CC the events to the lobby, unless the lobby is already the peer
        packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(data->lobby, 2, packet);
    }
    serial_free(&ser);
    enet_host_flush(host);
}

// replay the game state, using the input logs from both sides
int rewind_and_replay(wtf *data, game_state *gs_current) {
    // first, find the last frame we have input from the other side
    // this will be our next checkpoint (as no events can come in before
    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    game_state *gs = data->gs_bak;
    game_state *gs_new = NULL;
    char buf[512];

    game_state *gs_old = omf_calloc(1, sizeof(game_state));
    game_state_clone(gs, gs_old);

    log_debug("current game ticks is %" PRIu32 ", stored game ticks are %" PRIu32 ", last tick is %" PRIu32,
              gs_current->int_tick - data->local_proposal, gs->int_tick - data->local_proposal,
              data->last_tick - data->local_proposal);

    // fix the game state pointers in the controllers
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *gp = game_state_get_player(gs, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            c->gs = gs;
        }
    }

    uint64_t replay_start = SDL_GetTicks64();
    int tick_count = 0;

    uint32_t arena_hash = 0; // arena_state_hash(gs);

    uint32_t last_agreed = min2(data->last_acked_tick, data->last_received_tick);

    foreach(it, ev) {
        if(ev->tick + data->local_proposal <= data->gs_bak->int_tick) {
            // tick too old to matter
            list_delete(transcript, &it);
            continue;
        }

        // The next tick is past when we have agreement, so we need to save the last known good game state
        // for future replays
        if(gs_new == NULL && ev->tick > last_agreed && gs->int_tick - data->local_proposal <= last_agreed &&
           gs->int_tick > gs_old->int_tick) {
            log_debug("saving game state at last agreed on tick %d with hash %" PRIu32,
                      gs->int_tick - data->local_proposal, arena_state_hash(gs));
            // save off the game state at the point we last agreed
            // on the state of the game
            gs_new = omf_calloc(1, sizeof(game_state));
            game_state_clone(gs, gs_new);
            data->gs_bak = gs_new;
            game_state_clone_free(gs_old);
            omf_free(gs_old);
        }

        // tick the number of required times
        while(gs->int_tick - data->local_proposal < ev->tick) {
            // Tick scene
            game_state_dynamic_tick(gs, true);
            arena_hash = arena_state_hash(gs);
            tick_count++;
            if(data->trace_file && ev->tick <= last_agreed &&
               gs->int_tick - data->local_proposal > data->last_traced_tick &&
               gs->int_tick - data->local_proposal < ev->tick) {
                data->last_traced_tick = gs->int_tick - data->local_proposal;
                int sz = snprintf(buf, sizeof(buf), "tick %d -- player 1 _ -- player 2 _ -- hash %" PRIu32 "\n",
                                  gs->int_tick - data->local_proposal, arena_hash);
                SDL_RWwrite(data->trace_file, buf, sz, 1);
                arena_state_dump(gs, buf, sizeof(buf));
                SDL_RWwrite(data->trace_file, buf, strlen(buf), 1);
            }
        }

        assert(gs->int_tick - data->local_proposal == ev->tick);

        // feed in the inputs
        for(int j = 0; j < 2; j++) {
            int player_id = j;
            game_player *player = game_state_get_player(gs, player_id);
            int k = 0;
            do {
                object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ev->events[j][k]);
                k++;
            } while(ev->events[j][k]);
        }

        arena_hash = arena_state_hash(gs);

        if((ev->events[0][0] || ev->events[1][0]) && ev->tick <= last_agreed && ev->tick > data->last_traced_tick) {
            // this event has been agreed on by both sides
            sd_rec_move move;
            data->last_traced_tick = ev->tick;

            for(int j = 0; j < 2; j++) {
                move.tick = ev->tick;
                move.lookup_id = 2;
                move.player_id = j;
                move.action = 0;

                int k = 0;
                while(ev->events[j][k]) {
                    if(ev->events[j][k] & ACT_PUNCH) {
                        move.action |= SD_ACT_PUNCH;
                    }

                    if(ev->events[j][k] & ACT_KICK) {
                        move.action |= SD_ACT_KICK;
                    }

                    if(ev->events[j][k] & ACT_UP) {
                        move.action |= SD_ACT_UP;
                    }

                    if(ev->events[j][k] & ACT_DOWN) {
                        move.action |= SD_ACT_DOWN;
                    }

                    if(ev->events[j][k] & ACT_LEFT) {
                        move.action |= SD_ACT_LEFT;
                    }

                    if(ev->events[j][k] & ACT_RIGHT) {
                        move.action |= SD_ACT_RIGHT;
                    }

                    if(ev->events[j][k] == ACT_NONE) {
                        move.action = SD_ACT_NONE;
                    }

                    sd_rec_insert_action(gs->rec, gs->rec->move_count, &move);
                    k++;
                }
            }

            if(data->trace_file) {
                char buf0[12];
                char buf1[12];

                event_names(buf0, ev->events[0]);
                event_names(buf1, ev->events[1]);

                int sz =
                    snprintf(buf, sizeof(buf), "tick %d -- player 1 %s (%d) -- player 2 %s (%d) -- hash %" PRIu32 "\n",
                             ev->tick, buf0, ev->events[0][0], buf1, ev->events[1][0], arena_hash);
                SDL_RWwrite(data->trace_file, buf, sz, 1);
                arena_state_dump(gs, buf, sizeof(buf));
                SDL_RWwrite(data->trace_file, buf, strlen(buf), 1);
            }
        }

        if(gs->int_tick - data->local_proposal == data->peer_last_hash_tick && data->peer_last_hash != arena_hash &&
           ev->tick <= last_agreed) {
            if(data->trace_file) {
                int sz = snprintf(buf, sizeof(buf), "---MISMATCH at %d (%d) got %" PRIu32 " expected %" PRIu32 "\n",
                                  gs->int_tick - data->local_proposal, data->peer_last_hash_tick, data->peer_last_hash,
                                  arena_hash);
                SDL_RWwrite(data->trace_file, buf, sz, 1);

                char buf0[12];
                char buf1[12];

                event_names(buf0, ev->events[0]);
                event_names(buf1, ev->events[1]);

                sz = snprintf(buf, sizeof(buf), "tick %d -- player 1 %s (%d) -- player 2 %s (%d) -- hash %" PRIu32 "\n",
                              ev->tick, buf0, ev->events[0][0], buf1, ev->events[1][0], arena_hash);
                SDL_RWwrite(data->trace_file, buf, sz, 1);
                arena_state_dump(gs, buf, sizeof(buf));
                SDL_RWwrite(data->trace_file, buf, strlen(buf), 1);
            }

            log_debug("arena hash mismatch at %d (%d) -- got %" PRIu32 " expected %" PRIu32 "!",
                      gs->int_tick - data->local_proposal, data->peer_last_hash_tick, data->peer_last_hash, arena_hash);
            for(int i = 0; i < game_state_num_players(gs); i++) {
                game_player *gp = game_state_get_player(gs, i);
                controller *c = game_player_get_ctrl(gp);
                if(c) {
                    c->gs = gs_current;
                }
            }
            return 1;
        } else if(gs->int_tick - data->local_proposal == data->peer_last_hash_tick) {
            log_debug("arena hashes agree!");
        }

        if(ev->tick <= last_agreed && data->last_hash_tick < gs->int_tick - data->local_proposal) {
            data->last_hash_tick = gs->int_tick - data->local_proposal;
            data->last_hash = arena_hash;
        }

        // controller_cmd(ctrl, action, ev);
    }

    log_debug("game state is %" PRIu32 ", want %" PRIu32, gs->int_tick - data->local_proposal,
              data->last_tick - data->local_proposal);
    uint32_t ticks = data->last_tick - gs->int_tick;
    // tick the number of required times
    for(int dynamic_wait = (int)ticks; dynamic_wait > 0; dynamic_wait--) {
        // Tick scene
        game_state_dynamic_tick(gs, true);
    }

    uint64_t replay_end = SDL_GetTicks64();

    if(gs_new == NULL) {
        // we weren't able to make a new state backup, so restore the old one
        data->gs_bak = gs_old;
    }

    log_debug("advanced game state to %" PRIu32 ", expected %" PRIu32, gs->int_tick - data->local_proposal,
              data->last_tick - data->local_proposal);

    log_debug("replayed %d ticks in %d milliseconds", tick_count, replay_end - replay_start);

    // replace the game state with the replayed one
    gs->new_state = NULL;
    if(gs_current->new_state) {
        game_state_clone_free(gs_current->new_state);
        omf_free(gs_current->new_state);
    }
    gs_current->new_state = gs;
    data->gs_bak->new_state = NULL;
    return 0;
}

ENetPeer *net_controller_get_lobby_connection(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->lobby;
}

ENetHost *net_controller_get_host(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->host;
}

void net_controller_set_winner(controller *ctrl, int winner) {
    wtf *data = ctrl->data;
    data->winner = winner;
}

int net_controller_get_winner(controller *ctrl) {
    wtf *data = ctrl->data;
    if(data->winner < 0) {
        return -1;
    }
    if(data->winner != data->id) {
        return 0;
    }
    return 1;
}

bool net_controller_ready(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->synchronized;
}

int net_controller_tick_offset(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->tick_offset / 2;
}

void net_controller_free(controller *ctrl) {
    wtf *data = ctrl->data;

    if(data->trace_file) {
        char buf[255];
        int sz = snprintf(buf, sizeof(buf), "------BEGIN TRANSCRIPT-------\n");

        SDL_RWwrite(data->trace_file, buf, sz, 1);

        iterator it;
        list_iter_begin(&data->transcript, &it);
        tick_events *ev = NULL;
        foreach(it, ev) {
            log_debug("tick %" PRIu32 " has events %d -- %d", ev->tick, ev->events[0], ev->events[1]);
            char buf0[12];
            char buf1[12];

            event_names(buf0, ev->events[0]);
            event_names(buf1, ev->events[1]);
            int sz = snprintf(buf, sizeof(buf), "tick %" PRIu32 " -- player 1 %s (%d) -- player 2 %s (%d)\n", ev->tick,
                              buf0, ev->events[0][0], buf1, ev->events[1][0]);
            SDL_RWwrite(data->trace_file, buf, sz, 1);
        }

        SDL_RWclose(data->trace_file);
    }
    ENetEvent event;
    if(!data->disconnected) {
        if(data->peer == data->lobby) {
            goto done;
        }
        log_debug("closing connection");
        enet_peer_disconnect(data->peer, 0);

        while(enet_host_service(data->host, &event, 3000) > 0) {
            switch(event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    log_debug("got disconnect notice");
                    event.peer->data = NULL;
                    // peer has acknowledged the disconnect
                    goto done;
                    break;
                default:
                    break;
            }
        }
    }
done:
    if(data->host && !data->lobby) {
        enet_host_destroy(data->host);
        data->host = NULL;
    }
    list_free(&data->transcript);
    if(data->gs_bak) {
        game_state_clone_free(data->gs_bak);
        omf_free(data->gs_bak);
    }
    if(ctrl->data) {
        omf_free(ctrl->data);
    }
}

int net_controller_tick(controller *ctrl, uint32_t ticks0, ctrl_event **ev) {
    ENetEvent event;
    wtf *data = ctrl->data;
    ENetHost *host = data->host;
    ENetPeer *peer = data->peer;
    serial ser;
    uint32_t ticks = ctrl->gs->int_tick;

    if(data->gs_bak && has_event(data, ticks - 1) && ticks > data->last_tick) {
        data->last_tick = ticks;
        send_events(data);
    }

    data->last_tick = ticks;

    if(ticks == data->local_proposal && !data->synchronized) {
        if(data->confirmed) {
            log_debug("time to start match: %" PRIu32, ticks);
            data->synchronized = true;
        } else {
            // proposal expired
            data->local_proposal = 0;
        }
    }

    if(data->local_proposal && ticks > data->local_proposal && !data->synchronized) {
        log_debug("missed synchronize tick %" PRIu32 " -- @ %" PRIu32, data->local_proposal, ticks);
    }

    if(data->gs_bak == NULL && data->disconnected == 0 && scene_is_arena(game_state_get_scene(ctrl->gs)) &&
       (ticks - data->local_proposal) % 7 == 0 &&
       game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, 1)))) {
        arena_reset(ctrl->gs->sc);
        data->gs_bak = omf_calloc(1, sizeof(game_state));
        game_state_clone(ctrl->gs, data->gs_bak);
        // bypass counter that tries to suppress input from previous scene
        data->gs_bak->sc->static_ticks_since_start = 25;
        log_debug("cloned game state at arena tick %d hash %" PRIu32, data->gs_bak->int_tick - data->local_proposal,
                  arena_state_hash(data->gs_bak));
        data->local_proposal = ticks; // reset the tick offset to the start of the match
        data->last_hash_tick = data->gs_bak->int_tick - data->local_proposal;
        data->last_hash = arena_state_hash(data->gs_bak);
    } else if(data->gs_bak != NULL && !scene_is_arena(game_state_get_scene(ctrl->gs))) {
        // changed scene and no longer need a game state backup, release it
        game_state_clone_free(data->gs_bak);
        omf_free(data->gs_bak);
        data->last_action = ACT_NONE;
        data->last_direction = OBJECT_FACE_NONE;
        data->synchronized = false;
        data->local_proposal = 0;
        data->peer_proposal = 0;
        data->confirmed = false;
        data->last_tick = 0;
        data->last_sent = 0;
        data->gs_bak = NULL;
        data->last_received_tick = 0;
        data->last_acked_tick = 0;
        data->last_har_state = -1;
        data->last_traced_tick = 0;
        data->peer_last_hash = 0;
        data->peer_last_hash_tick = 0;
        data->last_hash = 0;
        data->last_hash_tick = 0;

        list_free(&data->transcript);
        list_create(&data->transcript);
    }

    int last_received = 0;
    int has_received = 0;

    while(enet_host_service(host, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
                switch(serial_read_int8(&ser)) {
                    case EVENT_TYPE_ACTION: {
                        last_received = 0;
                        uint32_t last_acked = serial_read_uint32(&ser);
                        uint32_t peer_last_hash_tick = serial_read_uint32(&ser);
                        uint32_t peer_last_hash = serial_read_uint32(&ser);
                        uint32_t peerticks = serial_read_uint32(&ser);
                        int8_t peer_frame_advantage = serial_read_int8(&ser);

                        data->frame_advantage =
                            (ticks - data->local_proposal) - (peerticks + (avg_rtt(data->rttbuf, 100) / 2));

                        if(data->gs_bak && data->synchronized && data->frame_advantage > peer_frame_advantage + 1) {
                            log_debug("%d %d (%d) frame advantage %d > %d", ticks - data->local_proposal, peerticks,
                                      (avg_rtt(data->rttbuf, 100) / 2), data->frame_advantage, peer_frame_advantage);
                            ctrl->gs->delay = (data->frame_advantage - peer_frame_advantage) * 2;
                            data->gs_bak->delay = (data->frame_advantage - peer_frame_advantage) * 2;
                        } else {
                            ctrl->gs->delay = 0;
                            if(data->gs_bak) {
                                data->gs_bak->delay = 0;
                            }
                        }

                        for(size_t i = 18; i < event.packet->dataLength;) {
                            unsigned remote_tick = serial_read_uint32(&ser);
                            // dispatch keypress to scene
                            int action = 0;
                            int k = 0;
                            do {
                                action = serial_read_int8(&ser);
                                k++;

                                if(action) {
                                    if(data->synchronized && data->gs_bak) {
                                        if(remote_tick > data->last_received_tick) {
                                            insert_event(data, remote_tick, action, abs(data->id - 1),
                                                         OBJECT_FACE_NONE);
                                        }
                                        last_received = remote_tick;
                                        if(action != 0) {
                                            has_received = 1;
                                        }
                                    } else {
                                        controller_cmd(ctrl, action, ev);
                                    }
                                }

                            } while(action);
                            i += 4 + k;
                        }
                        if(data->synchronized && data->gs_bak) {
                            data->last_received_tick = max2(data->last_received_tick, last_received);
                            data->last_acked_tick = max2(data->last_acked_tick, last_acked);

                            if(peer_last_hash_tick > data->peer_last_hash_tick) {
                                data->peer_last_hash_tick = peer_last_hash_tick;
                                data->peer_last_hash = peer_last_hash;
                                log_debug("peer last hash is %" PRIu32 " %d, local is %d %" PRIu32,
                                          data->peer_last_hash_tick, data->peer_last_hash,
                                          data->gs_bak->int_tick - data->local_proposal,
                                          arena_state_hash(data->gs_bak));
                            }
                        }
                    } break;
                    case EVENT_TYPE_HB: {
                        // got a tick
                        int id = serial_read_int8(&ser);
                        if(id == data->id) {
                            // this is a reply to our own heartbeat, analyze it
                            uint32_t start = serial_read_int32(&ser);
                            uint32_t peerticks = serial_read_int32(&ser);
                            uint32_t peerguess = serial_read_int32(&ser);

                            if(udist(peerguess, ticks) < 2) {
                                data->guesses++;
                            } else {
                                if(!data->synchronized) {
                                    log_debug("peer %d @ %d guessed our ticks INcorrectly! %d %d %d, actually  %d", id,
                                              peerticks, start, peerguess, peerguess - start, ticks - start);
                                }
                                data->guesses--;
                            }
                            int newrtt = udist(ticks, start);
                            if(data->guesses >= 10 &&
                               ((data->id == ROLE_SERVER && ticks % 37 == 0) ||
                                (data->id == ROLE_CLIENT && ticks % 73 == 0)) &&
                               !data->synchronized && data->peer_proposal == 0) {
                                // we're synchronized on a stable connection, propose a time to start the match
                                data->peer_proposal = peerticks + (newrtt / 2) + 100;
                                data->local_proposal = ticks + 100;
                                uint32_t seed = time(NULL);
                                random_seed(&ctrl->gs->rand, seed);
                                log_debug("proposing peer start game at their time %" PRIu32 ", my time %" PRIu32
                                          ", seed %" PRIu32,
                                          data->peer_proposal, data->local_proposal, seed);
                                ENetPacket *start_packet;
                                serial start_ser;
                                serial_create(&start_ser);

                                serial_write_int8(&start_ser, EVENT_TYPE_PROPOSE_START);
                                serial_write_uint32(&start_ser, data->peer_proposal);
                                serial_write_uint32(&start_ser, data->local_proposal);
                                serial_write_uint32(&start_ser, seed);

                                start_packet = enet_packet_create(start_ser.data, serial_len(&start_ser),
                                                                  ENET_PACKET_FLAG_RELIABLE);
                                enet_peer_send(peer, 1, start_packet);
                                enet_host_flush(host);
                                serial_free(&start_ser);
                            }

                            // log_debug("RTT was %d ticks", newrtt);
                            data->rttbuf[data->rttpos++] = newrtt;
                            data->tick_offset = newrtt;
                            if(data->rttpos >= 100) {
                                data->rttpos = 0;
                                data->rttfilled = 1;
                            }
                            ctrl->rtt = peer->roundTripTime;
                            data->outstanding_hb = 0;
                            data->last_hb = ticks;
                        } else {
                            uint32_t peerticks = serial_read_uint32(&ser);

                            // a heartbeat from the peer, bounce it back with our tick and our prediction of the peer's
                            // tick
                            ENetPacket *packet;
                            // write our own ticks into it
                            if(peer) {
                                // log_debug("peer ticks are %d, adding guess of %d", peerticks, data->tick_offset * 2);
                                serial_write_uint32(&ser, ticks);
                                serial_write_uint32(&ser, peerticks + data->tick_offset);
                                packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
                                enet_peer_send(peer, 1, packet);
                                enet_host_flush(host);
                            }
                        }
                    } break;
                    case EVENT_TYPE_PROPOSE_START: {
                        uint32_t peer_proposal = serial_read_uint32(&ser);
                        uint32_t local_proposal = serial_read_uint32(&ser);
                        uint32_t seed = serial_read_uint32(&ser);
                        if(peer_proposal + data->tick_offset > ticks && data->peer_proposal == 0) {
                            log_debug("got peer proposal to start @ %" PRIu32 " (currently %" PRIu32 "), seed %" PRIu32,
                                      peer_proposal, ticks, seed);
                            data->local_proposal = peer_proposal;
                            data->peer_proposal = local_proposal;
                            data->confirmed = true;
                            random_seed(&ctrl->gs->rand, seed);

                            ENetPacket *start_packet;
                            serial start_ser;
                            serial_create(&start_ser);

                            log_debug("confirmed starting round at %" PRIu32 "", peer_proposal);
                            serial_write_int8(&start_ser, EVENT_TYPE_CONFIRM_START);
                            serial_write_uint32(&start_ser, peer_proposal);

                            start_packet =
                                enet_packet_create(start_ser.data, serial_len(&start_ser), ENET_PACKET_FLAG_RELIABLE);
                            enet_peer_send(peer, 1, start_packet);
                            enet_host_flush(host);
                            serial_free(&start_ser);
                        }
                    } break;
                    case EVENT_TYPE_CONFIRM_START: {
                        uint32_t peer_proposal = serial_read_uint32(&ser);
                        if(data->peer_proposal == peer_proposal) {
                            // peer has agreed to our proposal
                            log_debug("peer agreed to start at %" PRIu32 " (local %" PRIu32 "), currently %" PRIu32 "",
                                      peer_proposal, data->local_proposal, ticks);
                            data->confirmed = true;
                        }
                    } break;
                    default:
                        // Event type is unknown or we don't care about it
                        break;
                }
                serial_free(&ser);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                log_debug("peer disconnected!");
                data->disconnected = 1;
                event.peer->data = NULL;
                data->synchronized = false;
                if(data->gs_bak) {
                    game_state_clone_free(data->gs_bak);
                    omf_free(data->gs_bak);
                }
                if(data->lobby) {
                    data->winner = arena_is_over(ctrl->gs->sc);
                    // lobby will handle the controller
                    game_state_set_next(ctrl->gs, SCENE_LOBBY);
                } else {
                    controller_close(ctrl, ev);
                    game_state_set_next(ctrl->gs, SCENE_MENU);
                }
                return 1; // bail the fuck out
                break;
            default:
                break;
        }
    }

    // if the match is actually proceeding (don't rewind during round/fight animation)
    // AND we've received events then try a rewind/replay
    if(has_received) {
        if(rewind_and_replay(data, ctrl->gs)) {
            if(data->lobby == data->peer) {
                game_state_set_next(ctrl->gs, SCENE_LOBBY);
                return 1;
            }
            enet_peer_disconnect(data->peer, 0);
            return 0;
        }
    }
    if(data->last_acked_tick < (ticks - data->local_proposal) - 50 && has_received) {
        // if we haven't sent an event in a while, send a dummy event to force the peer to
        // rewind/replay
        log_debug("sending blank events in response");
        send_events(data);
    }

    unsigned tick_interval = 5;
    if(data->rttfilled) {
        tick_interval = 20;
    }

    if((data->last_hb == 0 || ticks - data->last_hb > tick_interval) || !data->outstanding_hb) {
        data->outstanding_hb = 1;
        if(peer) {
            ENetPacket *packet;
            serial ser;
            serial_create(&ser);

            serial_write_int8(&ser, EVENT_TYPE_HB);
            serial_write_int8(&ser, data->id);
            serial_write_uint32(&ser, ticks);

            packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
            serial_free(&ser);
            enet_peer_send(peer, 1, packet);
            enet_host_flush(host);
        } else {
            log_debug("peer is null~");
            data->disconnected = 1;
            controller_close(ctrl, ev);
        }
    }

    return 0;
}

void controller_hook(controller *ctrl, int action) {
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;

    game_player *player = game_state_get_player(ctrl->gs, data->id);
    object *har_obj = game_state_find_object(ctrl->gs, game_player_get_har_obj_id(player));
    int direction = OBJECT_FACE_NONE;
    if(har_obj) {
        har *har = object_get_userdata(har_obj);
        data->last_har_state = har->state;
        direction = object_get_direction(har_obj);
    }

    if(peer) {
        // log_debug("Local event %d at %d", action, data->last_tick - data->local_proposal);
        if(data->synchronized && data->gs_bak) {
            insert_event(data, ctrl->gs->int_tick - data->local_proposal /*+ (ctrl->rtt / 2)*/, action, data->id,
                         direction);
        } else {
            serial ser;
            ENetPacket *packet;
            serial_create(&ser);
            serial_write_int8(&ser, EVENT_TYPE_ACTION);
            serial_write_uint32(&ser, 0);
            serial_write_uint32(&ser, 0);
            serial_write_uint32(&ser, 0);
            serial_write_uint32(&ser, 0);
            serial_write_int8(&ser, 0);
            serial_write_uint32(&ser, udist(data->last_tick, data->local_proposal));
            serial_write_int8(&ser, action);
            serial_write_int8(&ser, 0);
            // non gameplay events are not repeated, so they need to be reliable
            packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
            serial_free(&ser);
            enet_peer_send(peer, 1, packet);
            enet_host_flush(host);
        }
    } else {
        log_debug("peer is null~");
    }
}

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer, ENetPeer *lobby, int id) {
    wtf *data = omf_calloc(1, sizeof(wtf));
    data->id = id;
    data->host = host;
    data->peer = peer;
    data->lobby = lobby; // this is null in a peer-to-peer game
    data->last_hb = 0;
    data->outstanding_hb = 0;
    data->disconnected = 0;
    data->rttpos = 0;
    data->tick_offset = 0;
    data->rttfilled = 0;
    data->synchronized = false;
    data->local_proposal = 0;
    data->peer_proposal = 0;
    data->confirmed = false;
    data->last_tick = 0;
    data->last_sent = 0;
    data->gs_bak = NULL;
    data->last_received_tick = 0;
    data->last_acked_tick = 0;
    data->last_har_state = -1;
    data->trace_file = NULL;
    data->last_traced_tick = 0;
    data->winner = -1;
    data->last_action = ACT_NONE;
    data->last_direction = OBJECT_FACE_NONE;
    char *trace_file = settings_get()->net.trace_file;
    if(trace_file) {
        data->trace_file = SDL_RWFromFile(trace_file, "w");
        if(!data->trace_file) {
            log_debug("failed to open trace file");
        }
    }
    list_create(&data->transcript);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->controller_hook = &controller_hook;
    ctrl->free_fun = &net_controller_free;
}
