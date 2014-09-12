#include "game/scenes/mainmenu/menu_net.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component net_header;
    component net_connect_button;
    component net_listen_button;
    component net_done_button;
    menu connect_menu;
    menu listen_menu;
    ENetHost *host;
} net_menu_data;

void menu_net_free(menu *menu) {
    net_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->net_header);
    textbutton_free(&local->net_connect_button);
    textbutton_free(&local->net_listen_button);
    textbutton_free(&local->net_done_button);
    menu_free(&local->connect_menu);
    menu_free(&local->listen_menu);
    free(local);
}

void menu_net_create(menu *menu) {
    net_menu_data *local = malloc(sizeof(net_menu_data));

    // Zero out host
    local->host = NULL;

    // connect menu
    menu_create(&local->connect_menu, 10, 80, 300, 50);
    menu_connect_create(&local->connect_menu);

    // listen menu
    menu_create(&local->listen_menu, 10, 80, 300, 50);
    menu_listen_create(&local->listen_menu);

    // Components
    textbutton_create(&local->net_header, &font_large, "NETWORK PLAY");
    textbutton_create(&local->net_connect_button, &font_large, "CONNECT TO SERVER");
    textbutton_create(&local->net_listen_button, &font_large, "START SERVER");
    textbutton_create(&local->net_done_button, &font_large, "DONE");
    menu_attach(&local->net_menu, &local->net_header, 33);
    menu_attach(&local->net_menu, &local->net_connect_button, 11);
    menu_attach(&local->net_menu, &local->net_listen_button, 55);
    menu_attach(&local->net_menu, &local->net_done_button, 11);

    local->net_listen_button.userdata = scene;
    local->net_listen_button.click = mainmenu_listen_for_connections;

    local->net_header.disabled = 1;
    menu_select(&local->net_menu, &local->net_connect_button);

    local->net_connect_button.userdata = scene;
    local->net_connect_button.click = mainmenu_enter_menu_connect;

    local->net_done_button.userdata = scene;
    local->net_done_button.click = mainmenu_prev_menu;

    menu_set_userdata(local);
    menu_set_free_cb(menu_net_free);
}
