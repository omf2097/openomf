#include "game/scenes/mainmenu/menu_configuration.h"
#include "game/scenes/mainmenu/menu_video.h"
#include "game/scenes/mainmenu/menu_input.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "audio/music.h"
#include "audio/sound.h"

void menu_config_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_config_music_slide(component *c, void *userdata, int pos) {
    music_set_volume(pos/10.0f);
}

void menu_config_sound_slide(component *c, void *userdata, int pos) {
    sound_set_volume(pos/10.0f);
}

void menu_config_mono_toggle(component *c, void *userdata, int pos) {
    music_reload();
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

component* menu_configuration_create(scene *s) {
    const char* mono_opts[] = {"OFF","ON"};

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
    menu_attach(menu, textslider_create_bind(&tconf, "SOUND", 10, 1, menu_config_sound_slide, NULL, &settings_get()->sound.sound_vol));
    menu_attach(menu, textslider_create_bind(&tconf, "MUSIC", 10, 1, menu_config_music_slide, NULL, &settings_get()->sound.music_vol));
    menu_attach(menu, textselector_create_bind_opts(&tconf, "MONO", menu_config_mono_toggle, NULL, &settings_get()->sound.music_mono, mono_opts, 2));
    menu_attach(menu, textbutton_create(&tconf, "DONE", COM_ENABLED, menu_config_done, NULL));
    return menu;
}
