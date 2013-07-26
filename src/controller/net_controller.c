#include "controller/net_controller.h"
#include "utils/log.h"

typedef struct wtf_t {
    ENetHost *host;
    ENetPeer *peer;
} wtf;

void net_controller_free(controller *ctrl) {
    wtf *data = ctrl->data;
    enet_peer_disconnect(data->peer, 0);
    // TODO empty the buffer
    free(data);
}

void net_controller_tick(controller *ctrl) {
    ENetEvent event;
    wtf *data = ctrl->data;
    ENetHost *host = data->host;
    if (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                DEBUG("got packet %s", event.packet->data);
                int action = atoi((char*)event.packet->data);
                controller_cmd(ctrl, action);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                DEBUG("peer disconnected!");
                break;
            default:
                break;
        }
    }
}

int net_controller_handle(controller *ctrl, SDL_Event *event) {
    return 1;
}

void hook(controller *ctrl, int action) {
    wtf *data = ctrl->data;
    ENetPeer *peer = data->peer;
    ENetHost *host = data->host;
    ENetPacket * packet;
    char buf[10];
    if (action != 10) {
        DEBUG("sending event %d", action);
    }
    sprintf(buf, "%d", action);
    packet = enet_packet_create(buf, strlen (buf) + 1, ENET_PACKET_FLAG_RELIABLE);
    if (peer) {
        enet_peer_send(peer, 0, packet);
        enet_host_flush (host);
    }
}

void net_controller_create(controller *ctrl, controller *other, ENetHost *host, ENetPeer *peer) {
    wtf *data = malloc(sizeof(wtf));
    data->host = host;
    data->peer = peer;
    ctrl->data = data;
    ctrl->type = CTRL_TYPE_NETWORK;
    ctrl->tick_fun = &net_controller_tick;
    ctrl->handle_fun = &net_controller_handle;
    controller_add_hook(other, &hook);
}
