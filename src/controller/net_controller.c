#include <math.h>
#include <stdio.h>

#include "controller/net_controller.h"
#include "game/game_state_type.h"
#include "game/utils/serial.h"
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
    list transcript;
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

void insert_event(list *transcript, int tick, uint16_t action, int id) {

    iterator it;
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

    while(enet_host_service(host, &event, 0) > 0) {
        switch(event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
                switch(serial_read_int8(&ser)) {
                    case EVENT_TYPE_ACTION: {
                        // dispatch keypress to scene
                        int action = serial_read_int16(&ser);
                        int remote_tick = serial_read_int32(&ser);

                        if (data->synchronized) {
                            insert_event(&data->transcript, remote_tick - data->peer_proposal, action, data->id);
                            print_transcript(&data->transcript);
                        }
                        DEBUG("Remote event %d at %d", action, remote_tick - data->peer_proposal);
                        controller_cmd(ctrl, action, ev);
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
                    case EVENT_TYPE_SYNC:
                        controller_sync(ctrl, &ser, ev);
                        break;
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

int net_controller_update(controller *ctrl, serial *original) {
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket *packet;

    if(peer) {
        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_SYNC);
        serial_write(&ser, original->data, original->len);
        packet = enet_packet_create(ser.data, serial_len(&ser), 0);
        serial_free(&ser);
        enet_peer_send(peer, 1, packet);
        enet_host_flush(host);
    } else {
        DEBUG("peer is null~");
    }

    return 0;
}

void controller_hook(controller *ctrl, int action) {
    serial ser;
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket *packet;
    if(action == ACT_STOP && data->last_action == ACT_STOP) {
        data->last_action = -1;
        return;
    }
    data->last_action = action;

    if(peer) {
        if (data->synchronized) {
            insert_event(&data->transcript, data->last_tick - data->local_proposal, action, data->id);
            print_transcript(&data->transcript);
        }
        DEBUG("Local event %d at %d", action, data->last_tick - data->local_proposal);
        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_ACTION);
        serial_write_int16(&ser, action);
        serial_write_int32(&ser, data->last_tick);
        /*DEBUG("controller hook fired with %d", action);*/
        /*sprintf(buf, "k%d", action);*/
        packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);
        enet_peer_send(peer, 1, packet);
        enet_host_flush(host);
    } else {
        DEBUG("peer is null~");
    }
}

void net_controller_har_hook(int action, void *cb_data) {
    controller *ctrl = cb_data;
    wtf *data = ctrl->data;
    serial ser;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket *packet;
    if(action == ACT_STOP && data->last_action == ACT_STOP) {
        data->last_action = -1;
        return;
    }
    if(action == ACT_FLUSH) {
        enet_host_flush(host);
        return;
    }
    data->last_action = action;
    if(peer) {
        if (data->synchronized) {
            insert_event(&data->transcript, data->last_tick - data->local_proposal, action, data->id);
            print_transcript(&data->transcript);
        }

        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_ACTION);
        serial_write_int16(&ser, action);
        serial_write_int32(&ser, data->last_tick);
        /*DEBUG("controller hook fired with %d", action);*/
        /*sprintf(buf, "k%d", action);*/
        packet = enet_packet_create(ser.data, serial_len(&ser), ENET_PACKET_FLAG_RELIABLE);
        serial_free(&ser);
        enet_peer_send(peer, 1, packet);
        /*enet_host_flush (host);*/
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
    list_create(&data->transcript);
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->update_fun = &net_controller_update;
    ctrl->controller_hook = &controller_hook;
    ctrl->free_fun = &net_controller_free;
}
