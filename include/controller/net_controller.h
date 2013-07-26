#ifndef _NET_CONTROLLER_H
#define _NET_CONTROLLER_H

#include "controller/controller.h"
#include <SDL2/SDL.h>
#include <enet/enet.h>

void net_controller_create(controller *ctrl, controller *other, ENetHost *host, ENetPeer *peer);
void net_controller_free(controller *ctrl);

#endif // _NET_CONTROLLER_H
