#include "game/scenes/mainmenu/menu_configuration.h"
#include "game/scenes/mainmenu/menu_video.h"
#include "game/scenes/mainmenu/menu_audio.h"
#include "game/scenes/mainmenu/menu_input.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"

void menu_config_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_enter_input_1(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_input_create(s, 1));
}

void menu_enter_input_2(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_input_create(s, 2));
}

void menu_enter_video(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_video_create(s));
}

void menu_enter_audio(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_audio_create(s));
}

component* menu_configuration_create(scene *s) {
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = color_create(0, 121, 0, 255);

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "CONFIGURATION"));
    menu_attach(menu, filler_create());
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&tconf, "PLAYER 1 INPUT", COM_ENABLED, menu_enter_input_1, s));
    menu_attach(menu, textbutton_create(&tconf, "PLAYER 2 INPUT", COM_ENABLED, menu_enter_input_2, s));
    menu_attach(menu, textbutton_create(&tconf, "VIDEO OPTIONS", COM_ENABLED, menu_enter_video, s));
    menu_attach(menu, textbutton_create(&tconf, "AUDIO OPTIONS", COM_ENABLED, menu_enter_audio, s));
    menu_attach(menu, textbutton_create(&tconf, "DONE", COM_ENABLED, menu_config_done, NULL));
    return menu;
}
