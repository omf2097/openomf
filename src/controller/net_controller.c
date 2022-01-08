#include <math.h>
#include <stdio.h>

#include "controller/net_controller.h"
#include "game/utils/serial.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct wtf_t {
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
} wtf;

// simple standard deviation calculation
float stddev(float average, int data[], int n) {
    float variance = 0.0f;
    for (int i = 0; i < n; i++) {
        variance = (data[i] - average) * (data[i] - average);
    }
    return sqrtf(variance / n);
}

// calculate average round trip time, ignoring outliers outside 1 standard deviation
int avg_rtt(int data[], int n) {
    float average = 0.0f;
    float result = 0.0f;
    int j = 0;
    for (int i = 0; i < n; i++) {
        average += data[i];
    }
    average = average / n;

    float sd = stddev(average, data, n);
    for (int i = 0; i < n; i++) {
        if (fabsf(data[i] - average) <= sd) {
            result += data[i];
            j++;
        }
    }
    if (j != 0)
        return trunc(result / j);
    return 0;
}

int net_controller_ready(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->rttfilled;
}

int net_controller_tick_offset(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->tick_offset;
}

void net_controller_free(controller *ctrl) {
    wtf *data = ctrl->data;
    ENetEvent event;
    if (!data->disconnected) {
        DEBUG("closing connection");
        enet_peer_disconnect(data->peer, 0);

        while (enet_host_service(data->host, &event, 3000) > 0) {
            switch (event.type) {
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
    if (data->host) {
        enet_host_destroy(data->host);
        data->host = NULL;
    }
    if (ctrl->data) {
        omf_free(ctrl->data);
    }
}

int net_controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    ENetEvent event;
    wtf *data = ctrl->data;
    ENetHost *host = data->host;
    ENetPeer *peer = data->peer;
    serial ser;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            serial_create_from(&ser, (const char *)event.packet->data, event.packet->dataLength);
            switch (serial_read_int8(&ser)) {
            case EVENT_TYPE_ACTION: {
                // dispatch keypress to scene
                int action = serial_read_int16(&ser);
                controller_cmd(ctrl, action, ev);
            } break;
            case EVENT_TYPE_HB: {
                // got a tick
                int id = serial_read_int8(&ser);
                if (id == data->id) {
                    int start = serial_read_int32(&ser);
                    int peerticks = serial_read_int32(&ser);
                    int newrtt = abs(start - ticks);
                    data->rttbuf[data->rttpos++] = newrtt;
                    if (data->rttpos >= 100) {
                        data->rttpos = 0;
                        data->rttfilled = 1;
                    }
                    if (data->rttfilled == 1) {
                        ctrl->rtt = avg_rtt(data->rttbuf, 100);
                        data->tick_offset = (peerticks + (ctrl->rtt / 2)) - ticks;
                        /*DEBUG("I am %d ticks away from server: %d %d", data->tick_offset, ticks,
                         * peerticks);*/
                    }
                    data->outstanding_hb = 0;
                    data->last_hb = ticks;
                } else {
                    // a heartbeat from the peer, bounce it back
                    ENetPacket *packet;
                    // write our own ticks into it
                    if (peer) {
                        serial_write_int32(&ser, ticks);
                        packet =
                            enet_packet_create(ser.data, ser.len, ENET_PACKET_FLAG_UNSEQUENCED);
                        enet_peer_send(peer, 0, packet);
                        enet_host_flush(host);
                    }
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
    if (data->rttfilled) {
        tick_interval = 20;
    }

    if ((data->last_hb == -1 || ticks - data->last_hb > tick_interval) || !data->outstanding_hb) {
        data->outstanding_hb = 1;
        if (peer) {
            ENetPacket *packet;
            serial ser;
            serial_create(&ser);
            serial_write_int8(&ser, EVENT_TYPE_HB);
            serial_write_int8(&ser, data->id);
            serial_write_int32(&ser, ticks);
            packet = enet_packet_create(ser.data, ser.len, ENET_PACKET_FLAG_UNSEQUENCED);
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

    if (peer) {
        serial ser;
        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_SYNC);
        serial_write(&ser, original->data, original->len);
        packet = enet_packet_create(ser.data, ser.len, 0);
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
    if (action == ACT_STOP && data->last_action == ACT_STOP) {
        data->last_action = -1;
        return;
    }
    data->last_action = action;

    if (peer) {
        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_ACTION);
        serial_write_int16(&ser, action);
        /*DEBUG("controller hook fired with %d", action);*/
        /*sprintf(buf, "k%d", action);*/
        packet = enet_packet_create(ser.data, ser.len, ENET_PACKET_FLAG_RELIABLE);
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
    if (action == ACT_STOP && data->last_action == ACT_STOP) {
        data->last_action = -1;
        return;
    }
    if (action == ACT_FLUSH) {
        enet_host_flush(host);
        return;
    }
    data->last_action = action;
    if (peer) {
        serial_create(&ser);
        serial_write_int8(&ser, EVENT_TYPE_ACTION);
        serial_write_int16(&ser, action);
        /*DEBUG("controller hook fired with %d", action);*/
        /*sprintf(buf, "k%d", action);*/
        packet = enet_packet_create(ser.data, ser.len, ENET_PACKET_FLAG_RELIABLE);
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
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->update_fun = &net_controller_update;
    ctrl->controller_hook = &controller_hook;
}
