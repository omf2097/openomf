#ifndef SPEC_CONTROLLER_H
#define SPEC_CONTROLLER_H

#include "controller/controller.h"

#include <enet/enet.h>

void spec_controller_create(controller *ctrl, int player, ENetHost *host, ENetPeer *lobby, hashmap *events);
void spec_controller_free(controller *ctrl);

ENetPeer *spec_controller_get_lobby_connection(controller *ctrl);
ENetHost *spec_controller_get_host(controller *ctrl);

#endif // SPEC_CONTROLLER_H
