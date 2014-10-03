#include "game/scenes/mainmenu/menu_net.h"
#include "game/scenes/mainmenu/menu_connect.h"
#include "game/scenes/mainmenu/menu_listen.h"

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

component* menu_net_create(scene *s) {
    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "NETWORK PLAY"));
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&font_large, "CONNECT TO SERVER", COM_ENABLED, menu_net_connect, s));
    menu_attach(menu, textbutton_create(&font_large, "START SERVER", COM_ENABLED, menu_net_listen, s));
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_net_done, NULL));
    return menu;
}
