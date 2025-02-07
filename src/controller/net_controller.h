#ifndef NET_CONTROLLER_H
#define NET_CONTROLLER_H

#include "controller/controller.h"
#include <SDL.h>
#include <enet/enet.h>

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer, ENetPeer *lobby, int id);
void net_controller_free(controller *ctrl);
int net_controller_get_rtt(controller *ctrl);
void net_controller_har_hook(int action, void *cb_data);

bool net_controller_ready(controller *ctrl);
int net_controller_tick_offset(controller *ctrl);

ENetPeer *net_controller_get_lobby_connection(controller *ctrl);

ENetHost *net_controller_get_host(controller *ctrl);
int net_controller_get_winner(controller *ctrl);
void net_controller_set_winner(controller *ctrl, int winner);

#endif // NET_CONTROLLER_H
