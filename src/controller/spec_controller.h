#ifndef SPEC_CONTROLLER_H
#define SPEC_CONTROLLER_H

#include "controller/controller.h"

#include <enet/enet.h>

void spec_controller_create(controller *ctrl, int player, ENetHost *host, ENetPeer *lobby, hashmap *events);
void spec_controller_free(controller *ctrl);

#endif // SPEC_CONTROLLER_H
