#include "controller/net_controller.h"
#include "utils/log.h"
#include <stdio.h>

typedef struct wtf_t {
    ENetHost *host;
    ENetPeer *peer;
    int last;
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

int net_controller_tick(controller *ctrl) {
    ENetEvent event;
    wtf *data = ctrl->data;
    int action;
    ENetHost *host = data->host;
    if (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                action = atoi((char*)event.packet->data);
                if (action != 10) {
                    DEBUG("got packet %s", event.packet->data);
                }
                har_parse_command(ctrl->har, (char*)event.packet->data);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DEBUG("peer disconnected!");
                data->disconnected = 1;
                return 1; // bail the fuck out
                break;
            default:
                break;
        }
    }
    return 0;
}

int net_controller_handle(controller *ctrl, SDL_Event *event) {
    return 1;
}

void hook(char* buf, void *userdata) {
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

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer) {
    wtf *data = malloc(sizeof(wtf));
    data->host = host;
    data->peer = peer;
    data->last = -1;
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->handle_fun = &net_controller_handle;
    ctrl->har_hook = &hook;
}
