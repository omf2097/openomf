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
    int last_action;
    uint32_t outstanding_hb;
    int disconnected;
    int rttbuf[100];
    int rttpos;
    int rttfilled;
    int tick_offset;
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
    SDL_RWops *trace_file;
    game_state *gs_bak;
} wtf;

typedef struct {
    uint32_t tick;
    uint16_t events[2];
    uint8_t seen_peer;
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
    // DEBUG("sum %f, stddev %f, average %f", sum, sd, average);
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

void insert_event(wtf *data, uint32_t tick, uint16_t action, int id) {

    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    tick_events *nev = NULL;
    tick_events event;
    event.tick = tick;
    event.events[id] = action;
    event.events[abs(id - 1)] = 0;
    event.seen_peer = 1 << id;
    int i = 0;

    while((ev = (tick_events *)list_iter_next(&it))) {
        if(i == 0 && ev->tick > tick) {
            event.seen_peer |= ev->seen_peer;
            list_prepend(transcript, &event, sizeof(tick_events));
            return;
        } else if(ev->tick == tick) {
            ev->events[id] |= action;
            ev->seen_peer |= 1 << id;
            return;
        } else if(ev->tick < tick) {
            // even though we have no events for this tick, mark the peer has events past here
            ev->seen_peer |= 1 << id;
        }
        nev = list_iter_peek(&it);
        if(ev->tick < tick && nev && nev->tick > tick) {
            event.seen_peer |= nev->seen_peer;
            list_iter_append(&it, &event, sizeof(tick_events));
            return;
        }
        i++;
    }
    // either the list is empty, or this tick is later than anything else
    list_append(transcript, &event, sizeof(tick_events));
}

bool has_event(wtf *data, uint32_t tick) {

    iterator it;
    list_iter_begin(&data->transcript, &it);
    tick_events *ev = NULL;
    while((ev = (tick_events *)list_iter_next(&it))) {
        if(ev->tick == tick - data->local_proposal && ev->events[data->id]) {
            return true;
        }
    }
    return false;
}

void print_transcript(list *transcript) {
    iterator it;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    while((ev = (tick_events *)list_iter_next(&it))) {
        DEBUG("tick %d has events %d -- %d (%d)", ev->tick, ev->events[0], ev->events[1], ev->seen_peer);
    }
}

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

    int events = 0;

    while((ev = (tick_events *)list_iter_next(&it))) {
        if(ev->events[data->id] != 0 && ev->tick > data->last_acked_tick &&
           ev->tick < data->last_tick - data->local_proposal) {
            serial_write_int16(&ser, ev->events[data->id]);
            serial_write_uint32(&ser, ev->tick);
            events++;
        }
    }

    if(!events) {
        // serial_write_int16(&ser, 0);
        // serial_write_int32(&ser, data->last_tick - 10);
    }

    data->last_sent = data->last_tick;

    packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(peer, 1, packet);
    if(data->lobby && peer != data->lobby) {
        // CC the events to the lobby, unless the lobby is already the peer
        packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
        enet_peer_send(data->lobby, 1, packet);
    }
    serial_free(&ser);
    enet_host_flush(host);
}

int rewind_and_replay(wtf *data, game_state *gs_current) {
    // first, find the last frame we have input from the other side
    // this will be our next checkpoint (as no events can come in before
    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    game_state *gs = data->gs_bak;
    game_state *gs_new = NULL;
    char buf[255];

    game_state *gs_old = omf_calloc(1, sizeof(game_state));
    game_state_clone(gs, gs_old);

    DEBUG("current game ticks is %" PRIu32 ", stored game ticks are %" PRIu32 ", last tick is %" PRIu32,
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

    // int gs_start = gs->int_tick - data->local_proposal;

    int last_seen_peer = 0;

    while((ev = (tick_events *)list_iter_next(&it))) {
        if(ev->tick + data->local_proposal < data->gs_bak->int_tick) {
            // tick too old to matter
            DEBUG("tick %" PRIu32 " is older than %" PRIu32, ev->tick, data->gs_bak->int_tick - data->local_proposal);
            assert(ev->seen_peer == 3);
            list_delete(transcript, &it);
            continue;
        }

        if(last_seen_peer && last_seen_peer != 3 && ev->seen_peer == 3) {
            print_transcript(transcript);
            assert(false);
        }
        last_seen_peer = ev->seen_peer;

        // XXX TODO disable this for now, for unknown reason
        if(false && gs_new == NULL && ev->tick > umin2(data->last_acked_tick, data->last_received_tick) &&
           ev->seen_peer == 3) {
            DEBUG("tick %" PRIu32 " is newer than last acked tick %" PRIu32, ev->tick, data->last_acked_tick);
            // save off the game state at the point we last agreed
            // on the state of the game
            gs_new = omf_calloc(1, sizeof(game_state));
            game_state_clone(gs, gs_new);
            data->gs_bak = gs_new;
            game_state_clone_free(gs_old);
            omf_free(gs_old);
        }
        // these are 'dynamic ticks'
        int ticks = (ev->tick + data->local_proposal) - gs->int_tick;

        // tick the number of required times
        for(int dynamic_wait = ticks; dynamic_wait > 0; dynamic_wait--) {
            // Tick scene
            game_state_dynamic_tick(gs, true);
            DEBUG("arena tick %" PRIu32 ", hash %" PRIu32, gs->int_tick - data->local_proposal, arena_state_hash(gs));
            arena_state_dump(gs);
            if(data->trace_file && (ev->events[0] || ev->events[1]) && ev->seen_peer == 3 &&
               ev->tick > data->last_traced_tick) {
                data->last_traced_tick = ev->tick;
                int sz = snprintf(buf, sizeof(buf), "tick %d -- player 1 %d -- player 2 %d -- hash %" PRIu32 "\n",
                                  ev->tick, ev->events[0], ev->events[1], arena_state_hash(gs));
                SDL_RWwrite(data->trace_file, buf, sz, 1);
            }

            if(gs->int_tick - data->local_proposal == data->peer_last_hash_tick &&
               data->peer_last_hash != arena_state_hash(gs) && ev->seen_peer == 3) {
                if(data->trace_file) {
                    int sz = snprintf(buf, sizeof(buf), "---MISMATCH at %d (%d) got %" PRIu32 " expected %" PRIu32 "\n",
                                      gs->int_tick - data->local_proposal, data->peer_last_hash_tick,
                                      data->peer_last_hash, arena_state_hash(gs));
                    SDL_RWwrite(data->trace_file, buf, sz, 1);
                }

                DEBUG("arena hash mismatch at %d (%d) -- got %" PRIu32 " expected %" PRIu32 "!",
                      gs->int_tick - data->local_proposal, data->peer_last_hash_tick, data->peer_last_hash,
                      arena_state_hash(gs));
                for(int i = 0; i < game_state_num_players(gs); i++) {
                    game_player *gp = game_state_get_player(gs, i);
                    controller *c = game_player_get_ctrl(gp);
                    if(c) {
                        c->gs = gs_current;
                    }
                }
                if(gs_old) {
                    game_state_clone_free(gs_old);
                    omf_free(gs_old);
                }
                return 1;
            } else if(gs->int_tick - data->local_proposal == data->peer_last_hash_tick) {
                DEBUG("arena hashes agree!");
            }
        }
        if(ev->seen_peer == 3 && data->last_hash_tick < gs->int_tick - data->local_proposal) {
            data->last_hash_tick = gs->int_tick - data->local_proposal;
            data->last_hash = arena_state_hash(gs);
        }

        // feed in the inputs
        // XXX this is a hack for now

        for(int j = 0; j < 2; j++) {
            int player_id = j;
            game_player *player = game_state_get_player(gs, player_id);
            if(ev->events[j]) {
                DEBUG("replaying input %d from player %d at tick %d %d -- peer_seen %d", ev->events[j], player_id,
                      ev->tick, gs->int_tick - data->local_proposal, ev->seen_peer);
                if(((ev->events[j] & ~ACT_KICK) & ~ACT_PUNCH) != 0) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)),
                               (ev->events[j] & ~ACT_KICK) & ~ACT_PUNCH);
                }
                if(ev->events[j] & ACT_PUNCH) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_PUNCH);
                } else if(ev->events[j] & ACT_KICK) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_KICK);
                }

                // write_rec_move(gs->sc, player, ev->events[j]);
                //} else {
                //    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_STOP);
            }
        }
        // controller_cmd(ctrl, action, ev);
    }

    if(gs_new == NULL && gs->int_tick - data->local_proposal <= data->last_acked_tick) {
        gs_new = omf_calloc(1, sizeof(game_state));
        game_state_clone(gs, gs_new);
        data->gs_bak = gs_new;
        game_state_clone_free(gs_old);
        omf_free(gs_old);
    } else if(gs_new == NULL) {
        data->gs_bak = gs_old;
    }

    DEBUG("game state is %" PRIu32 ", want %" PRIu32, gs->int_tick, data->last_tick);
    uint32_t ticks = data->last_tick - gs->int_tick;
    // tick the number of required times
    for(int dynamic_wait = (int)ticks; dynamic_wait > 0; dynamic_wait--) {
        // Tick scene
        game_state_dynamic_tick(gs, true);
    }

    DEBUG("advanced game state to %" PRIu32 ", expected %" PRIu32, gs->int_tick - data->local_proposal,
          data->last_tick - data->local_proposal);

    // replace the game state with the replayed one
    gs_current->new_state = gs;
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
        while((ev = (tick_events *)list_iter_next(&it))) {
            DEBUG("tick %" PRIu32 " has events %d -- %d", ev->tick, ev->events[0], ev->events[1]);
            int sz = snprintf(buf, sizeof(buf), "tick %" PRIu32 " -- player 1 %d -- player 2 %d -- seen_peer %d\n",
                              ev->tick, ev->events[0], ev->events[1], ev->seen_peer);
            SDL_RWwrite(data->trace_file, buf, sz, 1);
        }

        SDL_RWclose(data->trace_file);
    }
    ENetEvent event;
    if(!data->disconnected) {
        DEBUG("closing connection");
        enet_peer_disconnect(data->peer, 0);

        while(enet_host_service(data->host, &event, 3000) > 0) {
            switch(event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    DEBUG("got disconnect notice");
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

    if(has_event(data, ticks - 1) && ticks > data->last_tick) {
        DEBUG("sending events %d -- %d", ticks - data->local_proposal, data->last_acked_tick);
        send_events(data);
        data->last_tick = ticks;
        // if(rewind_and_replay(data, ctrl->gs)) {
        //     enet_peer_disconnect(data->peer, 0);
        //     return 0;
        // }
    }

    data->last_tick = ticks;

    if(ticks == data->local_proposal && !data->synchronized) {
        if(data->confirmed) {
            DEBUG("time to start match: %" PRIu32, ticks);
            data->synchronized = true;
        } else {
            // proposal expired
            data->local_proposal = 0;
        }
    }

    if(ticks > data->local_proposal && !data->synchronized) {
        DEBUG("missed synchronize tick %" PRIu32 " -- @ %" PRIu32, data->local_proposal, ticks);
    }

    if(data->gs_bak == NULL && is_arena(game_state_get_scene(ctrl->gs)->id) &&
       (ticks - data->local_proposal) % 7 == 0 &&
       game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, 1)))) {
        arena_reset(ctrl->gs->sc);
        data->gs_bak = omf_calloc(1, sizeof(game_state));
        game_state_clone(ctrl->gs, data->gs_bak);
        DEBUG("cloned game state at arena tick %d hash %" PRIu32, data->gs_bak->int_tick - data->local_proposal,
              arena_state_hash(data->gs_bak));
        data->local_proposal = ticks; // reset the tick offset to the start of the match
        data->last_hash_tick = data->gs_bak->int_tick - data->local_proposal;
        data->last_hash = arena_state_hash(data->gs_bak);
    } else if(data->gs_bak != NULL && !is_arena(game_state_get_scene(ctrl->gs)->id)) {
        // changed scene and no longer need a game state backup, release it
        game_state_clone_free(data->gs_bak);
        omf_free(data->gs_bak);
    }

    while(enet_host_service(host, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
                switch(serial_read_int8(&ser)) {
                    case EVENT_TYPE_ACTION: {
                        assert(event.packet->dataLength % 6 == 1);
                        uint32_t last_received = 0;
                        uint32_t last_acked = serial_read_uint32(&ser);
                        uint32_t peer_last_hash_tick = serial_read_uint32(&ser);
                        uint32_t peer_last_hash = serial_read_uint32(&ser);

                        for(size_t i = 13; i < event.packet->dataLength; i += 6) {
                            // dispatch keypress to scene
                            int action = serial_read_int16(&ser);
                            unsigned remote_tick = serial_read_uint32(&ser);

                            if(data->synchronized && data->gs_bak) {
                                DEBUG("inserting event %d at tick %" PRIu32, action, remote_tick);
                                if(remote_tick > data->last_received_tick) {
                                    insert_event(data, remote_tick, action, abs(data->id - 1));
                                }
                                last_received = remote_tick;
                                // print_transcript(&data->transcript);
                            } else {
                                DEBUG("Remote event %d at %" PRIu32, action, remote_tick);
                                controller_cmd(ctrl, action, ev);
                            }
                        }
                        if(data->synchronized && data->gs_bak) {
                            // print_transcript(&data->transcript);
                            data->last_received_tick = max2(data->last_received_tick, last_received);
                            data->last_acked_tick = max2(data->last_acked_tick, last_acked);
                            if(peer_last_hash_tick > data->peer_last_hash_tick) {
                                data->peer_last_hash_tick = peer_last_hash_tick;
                                data->peer_last_hash = peer_last_hash;
                                DEBUG("peer last hash is %" PRIu32 " %d, local is %d %" PRIu32,
                                      data->peer_last_hash_tick, data->peer_last_hash, data->gs_bak->int_tick,
                                      arena_state_hash(data->gs_bak));
                            }
                            if(last_received && rewind_and_replay(data, ctrl->gs)) {
                                enet_peer_disconnect(data->peer, 0);
                                return 0;
                            }
                            if(data->last_sent + data->local_proposal < ticks - 50) {
                                // if we haven't sent an event in a while, send a dummy event to force the peer to
                                // rewind/replay
                                send_events(data);
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
                                // DEBUG("peer @ %d guessed our ticks correctly! %d %d %d", peerticks, start, peerguess,
                                // peerguess - start);
                                data->guesses++;
                            } else {
                                if(!data->synchronized) {
                                    DEBUG("peer @ %d guessed our ticks INcorrectly! %d %d %d, actually  %d", peerticks,
                                          start, peerguess, peerguess - start, ticks - start);
                                }
                                data->guesses = 0;
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
                                DEBUG("proposing peer start game at their time %" PRIu32 ", my time %" PRIu32
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
                                                                  ENET_PACKET_FLAG_UNSEQUENCED);
                                enet_peer_send(peer, 0, start_packet);
                                enet_host_flush(host);
                                serial_free(&start_ser);
                                // enet_packet_destroy(start_packet);
                            }

                            // DEBUG("RTT was %d ticks", newrtt);
                            data->rttbuf[data->rttpos++] = newrtt;
                            data->tick_offset = newrtt;
                            if(data->rttpos >= 100) {
                                data->rttpos = 0;
                                data->rttfilled = 1;
                            }
                            if(data->rttfilled == 1) {
                                ctrl->rtt = avg_rtt(data->rttbuf, 100);
                                // data->tick_offset = (peerticks + (ctrl->rtt / 2)) - ticks;
                                // DEBUG("I am %d ticks away from server: %d %d RTT %d", data->tick_offset, ticks,
                                // peerticks, ctrl->rtt);
                            }
                            data->outstanding_hb = 0;
                            data->last_hb = ticks;
                        } else {
                            uint32_t peerticks = serial_read_uint32(&ser);

                            // a heartbeat from the peer, bounce it back with our tick and our prediction of the peer's
                            // tick
                            ENetPacket *packet;
                            // write our own ticks into it
                            if(peer) {
                                // DEBUG("peer ticks are %d, adding guess of %d", peerticks, data->tick_offset * 2);
                                serial_write_uint32(&ser, ticks);
                                serial_write_uint32(&ser, peerticks + data->tick_offset);
                                packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
                                enet_peer_send(peer, 0, packet);
                                enet_host_flush(host);
                            }
                        }
                    } break;
                    case EVENT_TYPE_PROPOSE_START: {
                        uint32_t peer_proposal = serial_read_uint32(&ser);
                        uint32_t local_proposal = serial_read_uint32(&ser);
                        uint32_t seed = serial_read_uint32(&ser);
                        if(peer_proposal + data->tick_offset > ticks && data->peer_proposal == 0) {
                            DEBUG("got peer proposal to start @ %" PRIu32 " (currently %" PRIu32 "), seed %" PRIu32,
                                  peer_proposal, ticks, seed);
                            data->local_proposal = peer_proposal;
                            data->peer_proposal = local_proposal;
                            data->confirmed = true;
                            random_seed(&ctrl->gs->rand, seed);

                            ENetPacket *start_packet;
                            serial start_ser;
                            serial_create(&start_ser);

                            DEBUG("confirmed starting round at %" PRIu32 "", peer_proposal);
                            serial_write_int8(&start_ser, EVENT_TYPE_CONFIRM_START);
                            serial_write_uint32(&start_ser, peer_proposal);

                            start_packet = enet_packet_create(start_ser.data, serial_len(&start_ser),
                                                              ENET_PACKET_FLAG_UNSEQUENCED);
                            enet_peer_send(peer, 0, start_packet);
                            enet_host_flush(host);
                            serial_free(&start_ser);
                            // enet_packet_destroy(start_packet);
                        }
                    } break;
                    case EVENT_TYPE_CONFIRM_START: {
                        uint32_t peer_proposal = serial_read_uint32(&ser);
                        if(data->peer_proposal == peer_proposal) {
                            // peer has agreed to our proposal
                            DEBUG("peer agreed to start at %" PRIu32 " (local %" PRIu32 "), currently %" PRIu32 "",
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
                DEBUG("peer disconnected!");
                data->disconnected = 1;
                event.peer->data = NULL;
                data->synchronized = false;
                if(data->gs_bak) {
                    game_state_clone_free(data->gs_bak);
                    omf_free(data->gs_bak);
                }
                controller_close(ctrl, ev);
                return 1; // bail the fuck out
                break;
            default:
                break;
        }
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
            enet_peer_send(peer, 0, packet);
            enet_host_flush(host);
        } else {
            DEBUG("peer is null~");
            data->disconnected = 1;
            controller_close(ctrl, ev);
        }
    }

    /*if(!handled) {*/
    /*controller_cmd(ctrl, ACT_STOP, ev);*/
    /*}*/
    return 0;
}

void controller_hook(controller *ctrl, int action) {
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    // if(action == ACT_STOP && data->last_action == ACT_STOP) {
    // data->last_action = -1;

    game_player *player = game_state_get_player(ctrl->gs, data->id);
    object *har_obj = game_state_find_object(ctrl->gs, game_player_get_har_obj_id(player));
    if(har_obj) {
        har *har = object_get_userdata(har_obj);
        if(data->last_action == action && har->state == data->last_har_state) {
            return;
        }
        int arena_state = arena_get_state(game_state_get_scene(ctrl->gs));
        if(arena_state != ARENA_STATE_FIGHTING) {
            return;
        }
        data->last_har_state = har->state;
        data->last_action = action;
    }

    if(peer) {
        // DEBUG("Local event %d at %d", action, data->last_tick - data->local_proposal);
        if(data->synchronized && data->gs_bak) {
            DEBUG("inserting event %d at tick %" PRIu32, action, ctrl->gs->int_tick - data->local_proposal);
            insert_event(data, ctrl->gs->int_tick - data->local_proposal, action, data->id);
            // print_transcript(&data->transcript);
            // send_events(data);
            // rewind_and_replay(data, ctrl->gs);
        } else {
            serial ser;
            ENetPacket *packet;
            serial_create(&ser);
            serial_write_int8(&ser, EVENT_TYPE_ACTION);
            serial_write_uint32(&ser, 0);
            serial_write_uint32(&ser, 0);
            serial_write_uint32(&ser, 0);
            serial_write_int16(&ser, action);
            serial_write_uint32(&ser, udist(data->last_tick, data->local_proposal));
            DEBUG("controller hook fired with %d", action);
            /*sprintf(buf, "k%d", action);*/
            // non gameplay events are not repeated, so they need to be reliable
            packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
            serial_free(&ser);
            enet_peer_send(peer, 1, packet);
            enet_host_flush(host);
        }
    } else {
        DEBUG("peer is null~");
    }
}

void net_controller_har_hook(int action, void *cb_data) {
    controller *ctrl = cb_data;
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;

    if(action == ACT_STOP && data->last_action == ACT_STOP) {
        // data->last_action = -1;
        return;
    }
    data->last_action = action;
    if(peer) {
        DEBUG("har hook!");
        if(data->synchronized && data->gs_bak) {
            insert_event(data, data->last_tick - data->local_proposal, action, data->id);
            send_events(data);
            // print_transcript(&data->transcript);
        }
    } else {
        DEBUG("peer is null~");
    }
}

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer, ENetPeer *lobby, int id) {
    wtf *data = omf_calloc(1, sizeof(wtf));
    data->id = id;
    data->host = host;
    data->peer = peer;
    data->lobby = lobby; // this is null in a peer-to-peer game
    data->last_hb = 0;
    data->last_action = ACT_STOP;
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
    char *trace_file = settings_get()->net.trace_file;
    if(trace_file) {
        data->trace_file = SDL_RWFromFile(trace_file, "w");
        if(!data->trace_file) {
            DEBUG("failed to open trace file");
        }
    }
    list_create(&data->transcript);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->controller_hook = &controller_hook;
    ctrl->free_fun = &net_controller_free;
}
