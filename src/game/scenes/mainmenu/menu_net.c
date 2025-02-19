#include "game/scenes/mainmenu/menu_net.h"
#include "game/scenes/mainmenu/menu_connect.h"
#include "game/scenes/mainmenu/menu_listen.h"
#include "game/scenes/mainmenu/menu_widget_ids.h"

#include "game/gui/gui.h"

void menu_net_done(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_net_lobby(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_LOBBY);
}

void menu_net_connect(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_connect_create(s));
}

void menu_net_listen(component *c, void *userdata) {
    scene *s = userdata;
    component *new_menu = menu_listen_create(s);
    if(new_menu != NULL) {
        menu_set_submenu(c->parent, new_menu);
    }
}

component *menu_net_create(scene *s) {
    component *menu = menu_create();
    menu_attach(menu, label_create_title("NETWORK PLAY"));
    menu_attach(menu, filler_create());

    component *lobby = button_create("LOBBY", "Join the OpenOMF network lobby to challenge other players.", false,
                                     false, menu_net_lobby, s);
    widget_set_id(lobby, NETWORK_LOBBY_BUTTON_ID);
    menu_attach(menu, lobby);

    component *connect = button_create("CONNECT TO SERVER", "Connect to a specified IP address to play an opponent.",
                                       false, false, menu_net_connect, s);
    widget_set_id(connect, NETWORK_CONNECT_BUTTON_ID);
    menu_attach(menu, connect);

    component *listen = button_create("START SERVER", "Start a server for other players to connect to.", false, false,
                                      menu_net_listen, s);
    widget_set_id(listen, NETWORK_LISTEN_BUTTON_ID);
    menu_attach(menu, listen);

    menu_attach(menu, button_create("DONE", "Return to main menu.", false, false, menu_net_done, NULL));
    return menu;
}
