#include "game/scenes/mainmenu/menu_net.h"
#include "game/scenes/mainmenu/menu_connect.h"
#include "game/scenes/mainmenu/menu_listen.h"
#include "game/scenes/mainmenu/menu_widget_ids.h"

#include "game/gui/gui.h"

void menu_net_done(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
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
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = COLOR_DARK_GREEN;

    component *menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "NETWORK PLAY"));
    menu_attach(menu, filler_create());
    component *connect = textbutton_create(&tconf, "CONNECT TO SERVER", COM_ENABLED, menu_net_connect, s);
    widget_set_id(connect, NETWORK_CONNECT_BUTTON_ID);
    menu_attach(menu, connect);
    component *listen = textbutton_create(&tconf, "START SERVER", COM_ENABLED, menu_net_listen, s);
    widget_set_id(listen, NETWORK_LISTEN_BUTTON_ID);
    menu_attach(menu, listen);
    menu_attach(menu, textbutton_create(&tconf, "DONE", COM_ENABLED, menu_net_done, NULL));
    return menu;
}
