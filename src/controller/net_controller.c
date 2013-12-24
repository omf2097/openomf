#include <stdio.h>

#include "controller/net_controller.h"
#include "utils/log.h"

typedef struct wtf_t {
    ENetHost *host;
    ENetPeer *peer;
    int last_hb;
    int outstanding_hb;
    int rtt;
    int disconnected;
} wtf;

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
                    // peer has acknowledged the disconnect
                    break;
                default:
                    break;
            }
        }
    }
    enet_host_destroy(data->host);
    free(data);
}

int net_controller_tick(controller *ctrl, int ticks, ctrl_event **ev) {
    ENetEvent event;
    wtf *data = ctrl->data;
    ENetHost *host = data->host;
    int handled = 0;
    if (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet->data[0] == 'k') {
                    // dispatch keypress to scene
                    int action = atoi((char*)event.packet->data+1);
                    controller_cmd(ctrl, action, ev);
                    handled = 1;
                } else if (event.packet->data[0] == 't') {
                    // got a tick
                    int start = atoi((char*)event.packet->data+1);
                    data->rtt = abs(start - ticks);
                    data->outstanding_hb = 0;
                    data->last_hb = ticks;
                } else {
                    serial *ser = malloc(sizeof(serial));
                    serial_create(ser);
                    ser->data = malloc(event.packet->dataLength);
                    ser->len = event.packet->dataLength;
                    memcpy(ser->data, event.packet->data, event.packet->dataLength);
                    controller_sync(ctrl, ser, ev);
                    handled = 1;
                }
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DEBUG("peer disconnected!");
                data->disconnected = 1;
                controller_close(ctrl, ev);
                return 1; // bail the fuck out
                break;
            default:
                break;
        }
    }
    if ((data->last_hb == -1 || ticks - data->last_hb > 10) && !data->outstanding_hb) {
        data->outstanding_hb = 1;
        char buf[10];
        ENetPeer *peer = data->peer;
        ENetPacket *packet;
        sprintf(buf, "t%d", ticks);
        packet = enet_packet_create(buf, strlen (buf) + 1, ENET_PACKET_FLAG_RELIABLE);
        if (peer) {
            enet_peer_send(peer, 0, packet);
            enet_host_flush (host);
        } else {
            DEBUG("peer is null~");
            data->disconnected = 1;
            controller_close(ctrl, ev);
        }
    }

    if(!handled) {
        controller_cmd(ctrl, ACT_STOP, ev);
    }
    return 0;
}

int net_controller_update(controller *ctrl, serial *serial) {
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket *packet;
    char *buf = malloc(serial->len);

    // need to copy here because enet will free this packet
    memcpy(buf, serial->data, serial->len);

    packet = enet_packet_create(buf, serial->len, ENET_PACKET_FLAG_RELIABLE);
    if (peer) {
        enet_peer_send(peer, 0, packet);
        enet_host_flush(host);
    } else {
        DEBUG("peer is null~");
    }

    return 0;
}

void har_hook(char* buf, void *userdata) {
    controller *ctrl = userdata;
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    data->disconnected = 0;
    ENetPacket * packet;

    packet = enet_packet_create(buf, strlen (buf) + 1, ENET_PACKET_FLAG_RELIABLE);
    if (peer) {
        enet_peer_send(peer, 0, packet);
        enet_host_flush (host);
    } else {
        DEBUG("peer is null~");
    }
}

void controller_hook(controller *ctrl, int action) {
    char buf[10];
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket *packet;
    if (action == ACT_STOP) {
        // not interested
        return;
    }
    /*DEBUG("controller hook fired with %d", action);*/
    sprintf(buf, "k%d", action);
    packet = enet_packet_create(buf, strlen (buf) + 1, ENET_PACKET_FLAG_RELIABLE);
    if (peer) {
        enet_peer_send(peer, 0, packet);
        enet_host_flush (host);
    } else {
        DEBUG("peer is null~");
    }
}

int net_controller_get_rtt(controller *ctrl) {
    wtf *data = ctrl->data;
    return data->rtt;
}

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer) {
    wtf *data = malloc(sizeof(wtf));
    data->host = host;
    data->peer = peer;
    data->last_hb = -1;
    data->outstanding_hb = 0;
    data->rtt = 0;
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->update_fun = &net_controller_update;
    ctrl->har_hook = &har_hook;
    ctrl->controller_hook = &controller_hook;
}
