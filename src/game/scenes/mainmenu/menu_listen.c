#include "game/scenes/mainmenu/menu_listen.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component listen_button;
    component listen_cancel_button;
} listen_menu_data;

void menu_listen_free(menu *menu) {
    listen_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->listen_button);
    textbutton_free(&local->listen_cancel_button);
    free(local);
}

void menu_listen_create(menu *menu) {
    listen_menu_data *local = malloc(sizeof(listen_menu_data));

    textbutton_create(&local->listen_button, &font_large, "Waiting for connection...");
    textbutton_create(&local->listen_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->listen_menu, &local->listen_button, 11);
    menu_attach(&local->listen_menu, &local->listen_cancel_button, 11);
    local->listen_button.disabled = 1;
    menu_select(&local->listen_menu, &local->listen_cancel_button);

    local->listen_cancel_button.userdata = scene;
    local->listen_cancel_button.click = mainmenu_cancel_connection;

    menu_set_userdata(local);
    menu_set_free_cb(menu_listen_free);
}
