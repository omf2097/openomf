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
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = TEXT_BRIGHT_GREEN;

    component *menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "CONFIGURATION"));
    menu_attach(menu, filler_create());

    component *language = textbutton_create(FONT_BIG, "LANGUAGE", "Forstar du ikke engelsk?", COM_ENABLED, menu_enter_language, s);
    component *player1 = textbutton_create(FONT_BIG, "PLAYER 1 INPUT", "Choose the control for player 1: keyboard or joystick.",
                                           COM_ENABLED, menu_enter_input_1, s);
    component *player2 = textbutton_create(FONT_BIG, "PLAYER 2 INPUT", "Choose the control for player 2: keyboard or joystick",
                                           COM_ENABLED, menu_enter_input_2, s);
    component *video = textbutton_create(FONT_BIG, "VIDEO OPTIONS", "Various options for visual effects and detail levels.",
                                         COM_ENABLED, menu_enter_video, s);
    component *audio = textbutton_create(FONT_BIG, "AUDIO OPTIONS", "Various options for audio effects and volume.",
                                         COM_ENABLED, menu_enter_audio, s);
    component *done = textbutton_create(FONT_BIG, "DONE", "Leave configuration.", COM_ENABLED, menu_config_done, NULL);

    menu_attach(menu, language);
    menu_attach(menu, player1);
    menu_attach(menu, player2);
    menu_attach(menu, video);
    menu_attach(menu, audio);
    menu_attach(menu, done);
    return menu;
}
