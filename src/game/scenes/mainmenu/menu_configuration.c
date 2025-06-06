#include "game/scenes/mainmenu/menu_configuration.h"
#include "game/scenes/mainmenu/menu_audio.h"
#include "game/scenes/mainmenu/menu_input.h"
#include "game/scenes/mainmenu/menu_language.h"
#include "game/scenes/mainmenu/menu_video.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"

void menu_config_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

static void menu_enter_language(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_language_create(s));
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

component *menu_configuration_create(scene *s) {
    component *menu = menu_create();
    menu_attach(menu, label_create_title("CONFIGURATION"));
    menu_attach(menu, filler_create());
    menu_attach(menu, button_create("LANGUAGE", "Forstar du ikke engelsk?", false, false, menu_enter_language, s));
    menu_attach(menu, button_create("PLAYER 1 INPUT", "Choose the control for player 1: keyboard or joystick.", false,
                                    false, menu_enter_input_1, s));
    menu_attach(menu, button_create("PLAYER 2 INPUT", "Choose the control for player 2: keyboard or joystick", false,
                                    false, menu_enter_input_2, s));
    menu_attach(menu, button_create("VIDEO OPTIONS", "Various options for visual effects and detail levels.", false,
                                    false, menu_enter_video, s));
    menu_attach(menu, button_create("AUDIO OPTIONS", "Various options for audio effects and volume.", false, false,
                                    menu_enter_audio, s));
    menu_attach(menu, button_create("DONE", "Leave configuration.", false, false, menu_config_done, NULL));
    return menu;
}
