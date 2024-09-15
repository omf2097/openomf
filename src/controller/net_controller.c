#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "controller/net_controller.h"
#include "game/game_state_type.h"
#include "game/utils/serial.h"
#include "game/protos/scene.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/list.h"
#include "utils/log.h"

typedef struct {
    ENetHost *host;
    ENetPeer *peer;
    int id;
    int last_hb;
    int last_action;
    int outstanding_hb;
    int disconnected;
    int rttbuf[100];
    int rttpos;
    int rttfilled;
    int tick_offset;
    bool synchronized;
    int guesses;
    int peer_proposal;
    int local_proposal;
    bool confirmed;
    int last_tick;
    int last_sent;
    list transcript;
    int last_received_tick;
    game_state *gs_bak;
} wtf;

typedef struct {
    int tick;
    uint16_t events[2];
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
    //DEBUG("sum %f, stddev %f, average %f", sum, sd, average);
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

void insert_event(wtf *data, int tick, uint16_t action, int id) {

    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    tick_events *nev = NULL;
    int i = 0;
    while((ev = (tick_events *)list_iter_next(&it))) {
        if (i == 0 && ev->tick > tick) {
            tick_events event;
            event.tick = tick;
            event.events[id] = action;
            event.events[abs(id - 1)] = 0;
            list_prepend(transcript, &event, sizeof(tick_events));
            return;
        } else if (ev->tick == tick) {
            ev->events[id] |= action;
            return;
        }
        nev = list_iter_peek(&it);
        if (ev->tick < tick && nev && nev->tick > tick) {
            tick_events event;
            event.tick = tick;
            event.events[id] = action;
            event.events[abs(id - 1)] = 0;
            list_iter_append(&it, &event, sizeof(tick_events));
            return;
        }
        i++;
    }
    // either the list is empty, or this tick is later than anything else
    tick_events event;
    event.tick = tick;
    event.events[id] = action;
    event.events[abs(id - 1)] = 0;
    list_append(transcript, &event, sizeof(tick_events));
}

void send_events(wtf * data) {
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

    while((ev = (tick_events *)list_iter_next(&it))) {
        if (ev->events[data->id] != 0 && ev->tick > data->last_received_tick && (ev->tick < data->last_tick || ev->events[data->id] == ACT_STOP)) {
            serial_write_int16(&ser, ev->events[data->id]);
            serial_write_int32(&ser, ev->tick);
        }
    }

    data->last_sent = data->last_tick;

    packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
    serial_free(&ser);
    enet_peer_send(peer, 1, packet);
    enet_host_flush(host);
}

void rewind_and_replay(wtf *data, game_state *gs_current) {
    // first, find the last frame we have input from the other side
    // this will be our next checkpoint (as no events can come in before
    iterator it;
    list *transcript = &data->transcript;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    game_state *gs = data->gs_bak;
    game_state *gs_new = NULL;

    DEBUG("current game ticks is %d, stored game ticks are %d, last tick is %d", gs_current->int_tick, gs->int_tick, data->last_tick);

    // fix the game state pointers in the controllers
    for(int i = 0; i < game_state_num_players(gs); i++) {
        game_player *gp = game_state_get_player(gs, i);
        controller *c = game_player_get_ctrl(gp);
        if(c) {
            c->gs = gs;
        }
    }

    while((ev = (tick_events *)list_iter_next(&it))) {
        if (ev->tick + data->local_proposal < gs->int_tick) {
            // tick too old to matter
            //DEBUG("tick %d is older than %d", ev->tick, gs->int_tick - data->local_proposal);
            list_delete(transcript, &it);
            continue;
        }

        if (gs_new == NULL && ev->tick >= data->last_received_tick) {
            DEBUG("tick %d is newer than last received tick %d", ev->tick, data->last_received_tick);
            // save off the game state at the point we last agreed
            // on the state of the game
            gs_new = omf_calloc(1, sizeof(game_state));
            game_state_clone(gs, gs_new);
            data->gs_bak = gs_new;
        }
        int ticks = (ev->tick + data->local_proposal) - gs->int_tick;

        int static_wait = (ticks * 10) / game_state_ms_per_dyntick(gs);
        int dynamic_wait = ticks;
        DEBUG("static wait %d, dynamic wait %d", static_wait, dynamic_wait);
        //int limit_static = 10;
        //int limit_dynamic = 10;
        while(static_wait > 1 /*&& limit_static--*/) {
            // Static tick for gamestate
            game_state_static_tick(gs, true);

            static_wait -= 1;
        }
        while(dynamic_wait > 1 /*&& limit_dynamic--*/) {
            // Tick scene
            game_state_dynamic_tick(gs, true);

            // Handle waiting period leftover time
            dynamic_wait -= 1;
        }

        // feed in the inputs
        // XXX this is a hack for now

        for (int j = 0; j < 2; j++) {
            int player_id = j;
            game_player *player = game_state_get_player(gs, player_id);
            if (ev->events[j]) {
                DEBUG("replaying input %d from player %d at tick %d", ev->events[j], player_id, ev->tick);
                if (((ev->events[j] & ~ACT_KICK) & ~ACT_PUNCH) != 0) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), (ev->events[j] & ~ACT_KICK) & ~ACT_PUNCH);
                }
                if (ev->events[j] & ACT_PUNCH) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_PUNCH);
                } else if (ev->events[j] & ACT_KICK) {
                    object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_KICK);
                }

                //write_rec_move(gs->sc, player, ev->events[j]);
            } else {
                object_act(game_state_find_object(gs, game_player_get_har_obj_id(player)), ACT_STOP);
            }
        }
        //controller_cmd(ctrl, action, ev);
    }

    if (gs_new == NULL) {
        gs_new = omf_calloc(1, sizeof(game_state));
        game_state_clone(gs, gs_new);
        data->gs_bak = gs_new;
    }

    DEBUG("game state is %d, want %d", gs->int_tick, data->last_tick); 
    int ticks = data->last_tick - gs->int_tick;
    int static_wait = (ticks * 10) / game_state_ms_per_dyntick(gs);
    int dynamic_wait = ticks;
    DEBUG("FINAL static wait %d, dynamic wait %d", static_wait, dynamic_wait);
    //int limit_static = 10;
    //int limit_dynamic = 10;
    while(static_wait > 1 /*&& limit_static--*/) {
        // Static tick for gamestate
        game_state_static_tick(gs, true);

        static_wait -= 1;
    }
    while(dynamic_wait > 1 /*&& limit_dynamic--*/) {
        // Tick scene
        game_state_dynamic_tick(gs, true);

        // Handle waiting period leftover time
        dynamic_wait -= 1;
    }


    DEBUG("advanced game state to %d, expected %d", gs->int_tick - data->local_proposal, data->last_tick - data->local_proposal);

    // replace the game state with the replayed one
    gs_current->new_state = gs;
}

void print_transcript(list *transcript) {
    iterator it;
    list_iter_begin(transcript, &it);
    tick_events *ev = NULL;
    while((ev = (tick_events *)list_iter_next(&it))) {
        DEBUG("tick %d has events %d -- %d", ev->tick, ev->events[0], ev->events[1]);
    }
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
    if(data->host) {
        enet_host_destroy(data->host);
        data->host = NULL;
    }
    list_free(&data->transcript);
    if (data->gs_bak) {
        game_state_clone_free(data->gs_bak);
        omf_free(data->gs_bak);
    }
    if(ctrl->data) {
        omf_free(ctrl->data);
    }
}

int net_controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    ENetEvent event;
    wtf *data = ctrl->data;
    ENetHost *host = data->host;
    ENetPeer *peer = data->peer;
    serial ser;

    data->last_tick = ticks;

    if (ticks == data->local_proposal && !data->synchronized) {
        if (data->confirmed) {
            DEBUG("time to start match: %d", ticks);
            data->synchronized = true;
        } else {
            // proposal expired
            data->local_proposal = 0;
        }
    }

    if (data->gs_bak == NULL && is_arena(game_state_get_scene(ctrl->gs)->id) && game_state_find_object(ctrl->gs, game_player_get_har_obj_id(game_state_get_player(ctrl->gs, 1)))) {
        data->gs_bak = omf_calloc(1, sizeof(game_state));
        game_state_clone(ctrl->gs, data->gs_bak);
    } else if (data->gs_bak != NULL && !is_arena(game_state_get_scene(ctrl->gs)->id)) {
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
                        int last_received = 0;
                        for (int i = 1; i < event.packet->dataLength; i+= 6) {
                            // dispatch keypress to scene
                            int action = serial_read_int16(&ser);
                            int remote_tick = serial_read_int32(&ser);

                            if (data->synchronized && data->gs_bak) {
                                insert_event(data, remote_tick, action, abs(data->id - 1));
                                last_received = remote_tick;
                                //print_transcript(&data->transcript);
                            } else {
                                DEBUG("Remote event %d at %d", action, remote_tick);
                                controller_cmd(ctrl, action, ev);
                            }
                        }
                        if (data->synchronized && data->gs_bak) {
                            //print_transcript(&data->transcript);
                            rewind_and_replay(data, ctrl->gs);
                            data->last_received_tick = last_received;
                            if (data->last_sent + data->local_proposal < ticks - 50) {
                                // if we haven't sent an event in a while, send a dummy event to force the peer to rewind/replay
                                serial action_ser;
                                ENetPacket *action_packet;
                                serial_create(&action_ser);
                                serial_write_int8(&action_ser, EVENT_TYPE_ACTION);
                                serial_write_int16(&action_ser, 0);
                                serial_write_int32(&action_ser, (ticks - data->local_proposal) - 1);
                                data->last_sent = (ticks - data->local_proposal) - 1;
                                action_packet = enet_packet_create(action_ser.data, serial_len(&action_ser), ENET_PACKET_FLAG_UNSEQUENCED);
                                serial_free(&action_ser);
                                enet_peer_send(peer, 1, action_packet);
                                enet_host_flush(host);
                            }
                        }
                    } break;
                    case EVENT_TYPE_HB: {
                        // got a tick
                        int id = serial_read_int8(&ser);
                        if(id == data->id) {
                            // this is a reply to our own heartbeat, analyze it
                            int start = serial_read_int32(&ser);
                            int peerticks  = serial_read_int32(&ser);
                            int peerguess = serial_read_int32(&ser);
                            if (abs(peerguess - ticks) < 2) {
                                //DEBUG("peer @ %d guessed our ticks correctly! %d %d %d", peerticks, start, peerguess, peerguess - start);
                                data->guesses++;
                            } else {
                                if (!data->synchronized) {
                                    DEBUG("peer @ %d guessed our ticks INcorrectly! %d %d %d, actually  %d", peerticks, start, peerguess, peerguess - start, ticks - start);
                                }
                                data->guesses = 0;
                            }
                            int newrtt = ticks - start;
                            if (data->guesses >= 10 && ((data->id == ROLE_SERVER && ticks % 37 == 0) || (data->id == ROLE_CLIENT && ticks % 73 == 0)) && !data->synchronized && data->peer_proposal == 0) {
                                // we're synchronized on a stable connection, propose a time to start the match
                                data->peer_proposal = peerticks + (newrtt / 2) + 50;
                                data->local_proposal = ticks + 50;
                                DEBUG("proposing peer start game at their time %d, my time %d", data->peer_proposal, data->local_proposal);
                                ENetPacket *start_packet;
                                serial start_ser;
                                serial_create(&start_ser);

                                serial_write_int8(&start_ser, EVENT_TYPE_PROPOSE_START);
                                serial_write_int32(&start_ser, data->peer_proposal);
                                serial_write_int32(&start_ser, data->local_proposal);


                                start_packet = enet_packet_create(start_ser.data, serial_len(&start_ser), ENET_PACKET_FLAG_UNSEQUENCED);
                                enet_peer_send(peer, 0, start_packet);
                                enet_host_flush(host);
                                serial_free(&start_ser);
                                //enet_packet_destroy(start_packet);
                            }

                            //DEBUG("RTT was %d ticks", newrtt);
                            data->rttbuf[data->rttpos++] = newrtt;
                            data->tick_offset = newrtt;
                            if(data->rttpos >= 100) {
                                data->rttpos = 0;
                                data->rttfilled = 1;
                            }
                            if(data->rttfilled == 1) {
                                ctrl->rtt = avg_rtt(data->rttbuf, 100);
                                //data->tick_offset = (peerticks + (ctrl->rtt / 2)) - ticks;
                                //DEBUG("I am %d ticks away from server: %d %d RTT %d", data->tick_offset, ticks, peerticks, ctrl->rtt);
                            }
                            data->outstanding_hb = 0;
                            data->last_hb = ticks;
                        } else {
                            int peerticks = serial_read_int32(&ser);

                            // a heartbeat from the peer, bounce it back with our tick and our prediction of the peer's tick
                            ENetPacket *packet;
                            // write our own ticks into it
                            if(peer) {
                                //DEBUG("peer ticks are %d, adding guess of %d", peerticks, data->tick_offset * 2);
                                serial_write_int32(&ser, ticks);
                                serial_write_int32(&ser, peerticks + data->tick_offset);
                                packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_UNSEQUENCED);
                                enet_peer_send(peer, 0, packet);
                                enet_host_flush(host);
                            }
                        }
                    } break;
                    case EVENT_TYPE_PROPOSE_START: {
                        int peer_proposal  = serial_read_int32(&ser);
                        int local_proposal  = serial_read_int32(&ser);
                        if (peer_proposal + data->tick_offset  > ticks && data->peer_proposal == 0) {
                            DEBUG("got peer proposal to start @ %d (currently %d)", peer_proposal, ticks);
                            data->local_proposal = peer_proposal;
                            data->peer_proposal = local_proposal;
                            data->confirmed = true;
                            ENetPacket *start_packet;
                            serial start_ser;
                            serial_create(&start_ser);

                            serial_write_int8(&start_ser, EVENT_TYPE_CONFIRM_START);
                            serial_write_int32(&start_ser, peer_proposal);


                            start_packet = enet_packet_create(start_ser.data, serial_len(&start_ser), ENET_PACKET_FLAG_UNSEQUENCED);
                            enet_peer_send(peer, 0, start_packet);
                            enet_host_flush(host);
                            serial_free(&start_ser);
                            //enet_packet_destroy(start_packet);
                        }
                    } break;
                    case EVENT_TYPE_CONFIRM_START: {
                        int peer_proposal  = serial_read_int32(&ser);
                        if (data->peer_proposal == peer_proposal) {
                            // peer has agreed to our proposal
                            DEBUG("peer agreed to start at %d (local %d), currently %d", peer_proposal, data->local_proposal, ticks);
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
                if (data->gs_bak) {
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

    int tick_interval = 5;
    if(data->rttfilled) {
        tick_interval = 20;
    }

    if((data->last_hb == -1 || ticks - data->last_hb > tick_interval) || !data->outstanding_hb) {
        data->outstanding_hb = 1;
        if(peer) {
            ENetPacket *packet;
            serial ser;
            serial_create(&ser);

            serial_write_int8(&ser, EVENT_TYPE_HB);
            serial_write_int8(&ser, data->id);
            serial_write_int32(&ser, ticks);

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
    if(action == ACT_STOP && data->last_action == ACT_STOP) {
        //data->last_action = -1;
    //if (data->last_action == action) {
        return;
    }
    data->last_action = action;

    if(peer) {
        //DEBUG("Local event %d at %d", action, data->last_tick - data->local_proposal);
        if (data->synchronized && data->gs_bak) {
            insert_event(data, data->last_tick - data->local_proposal, action, data->id);
            //print_transcript(&data->transcript);
            send_events(data);
            //rewind_and_replay(data, ctrl->gs);
        } else {
            serial ser;
            ENetPacket *packet;
            serial_create(&ser);
            serial_write_int8(&ser, EVENT_TYPE_ACTION);
            serial_write_int16(&ser, action);
            serial_write_int32(&ser, data->last_tick - data->local_proposal);
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
        //data->last_action = -1;
        return;
    }
    data->last_action = action;
    if(peer) {
        DEBUG("har hook!");
        if (data->synchronized && data->gs_bak) {
            insert_event(data, data->last_tick - data->local_proposal, action, data->id);
            send_events(data);
            //print_transcript(&data->transcript);
        }
    } else {
        DEBUG("peer is null~");
    }
}

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer, int id) {
    wtf *data = omf_calloc(1, sizeof(wtf));
    data->id = id;
    data->host = host;
    data->peer = peer;
    data->last_hb = -1;
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
    list_create(&data->transcript);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->controller_hook = &controller_hook;
    ctrl->free_fun = &net_controller_free;
}
