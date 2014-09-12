#include "game/scenes/mainmenu/menu_connect.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component connect_ip_input;
    component connect_ip_button;
    component connect_ip_cancel_button;
} connect_menu_data;

void menu_connect_free(menu *menu) {
    connect_menu_data *local = menu_get_userdata(menu);
    textinput_free(&local->connect_ip_input);
    textbutton_free(&local->connect_ip_button);
    textbutton_free(&local->connect_ip_cancel_button);
    free(local);
}

void menu_connect_create(menu *menu) {
    connect_menu_data *local = malloc(sizeof(connect_menu_data));

    textinput_create(&local->connect_ip_input, &font_large, "Host/IP", setting->net.net_connect_ip);
    textbutton_create(&local->connect_ip_button, &font_large, "CONNECT");
    textbutton_create(&local->connect_ip_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->connect_menu, &local->connect_ip_input, 11);
    menu_attach(&local->connect_menu, &local->connect_ip_button, 11);
    menu_attach(&local->connect_menu, &local->connect_ip_cancel_button, 11);

    local->connect_ip_button.userdata = scene;
    local->connect_ip_button.click = mainmenu_connect_to_ip;

    local->connect_ip_cancel_button.userdata = scene;
    local->connect_ip_cancel_button.click = mainmenu_cancel_connection;

    menu_set_userdata(local);
    menu_set_free_cb(menu_connect_free);
}
