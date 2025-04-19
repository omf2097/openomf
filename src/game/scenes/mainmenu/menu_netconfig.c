
#include "game/scenes/mainmenu/menu_netconfig.h"

#include "game/common_defines.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"

void menu_netconfig_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component *menu_netconfig_create(scene *s) {
    const char *bool_opts[] = {"OFF", "ON"};

    const char *input_delays[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    component *menu = menu_create();
    menu_attach(menu, label_create_title("NETWORK CONFIG"));
    menu_attach(menu, filler_create());
    menu_attach(menu, textselector_create_bind_opts(
                          "ENABLE NAT-PMP", "Enable this if your router has working NAT-PMP, disable otherwise.", NULL,
                          NULL, &settings_get()->net.net_use_pmp, bool_opts, 2));
    menu_attach(menu, textselector_create_bind_opts("ENABLE UPnP",
                                                    "Enable this if your router has working UPnP, disable otherwise.",
                                                    NULL, NULL, &settings_get()->net.net_use_upnp, bool_opts, 2));
    menu_attach(menu, textselector_create_bind_opts(
                          "INPUT DELAY", "Set input delay, in game ticks, for smoother rollback, default is 0", NULL,
                          NULL, &settings_get()->net.net_input_delay, input_delays, 11));

    menu_attach(menu, button_create("DONE", "Go back to the main menu.", false, false, menu_netconfig_done, NULL));
    return menu;
}
