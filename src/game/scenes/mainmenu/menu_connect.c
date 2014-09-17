#include "game/scenes/mainmenu/menu_connect.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "game/menu/filler.h"
#include "game/menu/label.h"
#include "game/menu/sizer.h"

#include "game/utils/settings.h"

typedef struct {

} connect_menu_data;

void menu_connect_free(component *c) {
    connect_menu_data *local = menu_get_userdata(c);
    free(local);
}

void menu_connect_tick(component *c) {

}

void menu_connect_start(component *c, void *userdata) {

}

void menu_connect_cancel(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component* menu_connect_create(scene *s) {
    connect_menu_data *local = malloc(sizeof(connect_menu_data));
    memset(local, 0, sizeof(connect_menu_data));

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "CONNECT TO SERVER"));
    menu_attach(menu, filler_create());

    menu_attach(menu, textinput_create(&font_large, "Host/IP", settings_get()->net.net_connect_ip));
    menu_attach(menu, textbutton_create(&font_large, "CONNECT", COM_ENABLED, menu_connect_start, s));
    menu_attach(menu, textbutton_create(&font_large, "CANCEL", COM_ENABLED, menu_connect_cancel, s));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_connect_free);
    menu_set_tick_cb(menu, menu_connect_tick);

    return menu;
}
