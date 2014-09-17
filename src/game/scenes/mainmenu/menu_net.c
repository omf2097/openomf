#include "game/scenes/mainmenu/menu_net.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "game/menu/filler.h"
#include "game/menu/label.h"
#include "game/menu/sizer.h"

typedef struct {
    ENetHost *host;
} net_menu_data;

void menu_net_free(component *c) {
    net_menu_data *local = menu_get_userdata(c);
    free(local);
}

void menu_net_done(component *c, void *userdata) {
    menu *m = menu_get_userdata(c->parent);
    m->finished = 1;
}

component* menu_net_create(scene *s) {
    net_menu_data *local = malloc(sizeof(net_menu_data));
    local->host = NULL;

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "NETWORK PLAY"));
    menu_attach(menu, filler_create());

    menu_attach(menu, textbutton_create(&font_large, "CONNECT TO SERVER", COM_ENABLED, NULL, NULL));
    menu_attach(menu, textbutton_create(&font_large, "START SERVER", COM_ENABLED, NULL, NULL));
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_net_done, NULL));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_net_free);

    return menu;
}
