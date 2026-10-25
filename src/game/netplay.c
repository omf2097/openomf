#include "game/netplay.h"

#if defined(ENABLE_NETPLAY)

#include "controller/net_controller.h"
#include "game/scenes/lobby.h"
#include "game/scenes/mainmenu/menu_connect.h"
#include "game/scenes/mainmenu/menu_listen.h"

#include <enet/enet.h>

bool netplay_enabled(void) {
    return true;
}

bool netplay_init(void) {
    return enet_initialize() == 0;
}

void netplay_quit(void) {
    enet_deinitialize();
}

int netplay_lobby_create(scene *scene) {
    return lobby_create(scene);
}

component *netplay_menu_connect_create(scene *s) {
    return menu_connect_create(s);
}

component *netplay_menu_listen_create(scene *s) {
    return menu_listen_create(s);
}

void netplay_set_winner(controller *ctrl, int winner) {
    net_controller_set_winner(ctrl, winner);
}

void netplay_menu_hook(controller *ctrl, int action) {
    menu_controller_hook(ctrl, action);
}

#else // !defined(ENABLE_NETPLAY)

#include "utils/log.h"

bool netplay_enabled(void) {
    return false;
}

bool netplay_init(void) {
    return true;
}

void netplay_quit(void) {
}

int netplay_lobby_create(scene *scene) {
    (void)scene;
    log_error("Network play is not available in this build.");
    return 1;
}

component *netplay_menu_connect_create(scene *s) {
    (void)s;
    return NULL;
}

component *netplay_menu_listen_create(scene *s) {
    (void)s;
    return NULL;
}

void netplay_set_winner(controller *ctrl, int winner) {
    (void)ctrl;
    (void)winner;
}

void netplay_menu_hook(controller *ctrl, int action) {
    (void)ctrl;
    (void)action;
}

#endif // !defined(ENABLE_NETPLAY)
