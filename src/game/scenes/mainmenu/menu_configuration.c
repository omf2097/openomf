#include "game/scenes/mainmenu/menu_configuration.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "game/menu/filler.h"
#include "game/menu/label.h"
#include "game/menu/sizer.h"

void menu_configuration_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

component* menu_configuration_create(scene *s) {
    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "CONFIGURATION"));
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&font_large, "PLAYER 1 INPUT", COM_ENABLED, NULL, NULL));
    menu_attach(menu, textbutton_create(&font_large, "PLAYER 2 INPUT", COM_ENABLED, NULL, NULL));
    menu_attach(menu, textbutton_create(&font_large, "VIDEO OPTIONS", COM_ENABLED, NULL, NULL));

    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_configuration_done, NULL));
    return menu;

    /*
    configuration_menu_data *local = malloc(sizeof(configuration_menu_data));

    // Create video menu
    menu_create(&local->video_menu, 165, 5, 151, 119);
    menu_video_create(&local->video_menu);

    // input config menu
    menu_create(&local->input_config_menu, 165, 5, 151, 119);
    menu_input_create(&local->input_config_menu);

    // input press key menu
    menu_create(&local->input_presskey_menu, 10, 80, 300, 20);
    menu_presskey_create(&local->input_presskey_menu);

    // Custom keyboard menu
    menu_create(&local->input_custom_keyboard_menu, 165, 5, 151, 119);
    menu_keyboard_create(&local->input_custom_keyboard_menu);

    // Configuration menu components
    textbutton_create(&local->config_header, &font_large, "CONFIGURATION");
    textbutton_create(&local->playerone_input_button, &font_large, "PLAYER 1 INPUT");
    textbutton_create(&local->playertwo_input_button, &font_large, "PLAYER 2 INPUT");
    textbutton_create(&local->video_options_button, &font_large, "VIDEO OPTIONS");
    textslider_create(&local->sound_slider, &font_large, "SOUND", 10, 1);
    textslider_create(&local->music_slider, &font_large, "MUSIC", 10, 1);
    textselector_create(&local->mono_toggle, &font_large, "MONO", "OFF");
    textselector_add_option(&local->mono_toggle, "ON");
    textbutton_create(&local->config_done_button, &font_large, "DONE");
    menu_attach(&local->config_menu, &local->config_header, 33);
    menu_attach(&local->config_menu, &local->playerone_input_button, 11);
    menu_attach(&local->config_menu, &local->playertwo_input_button, 11);
    menu_attach(&local->config_menu, &local->video_options_button, 11);
    menu_attach(&local->config_menu, &local->sound_slider, 11);
    menu_attach(&local->config_menu, &local->music_slider, 11);
    menu_attach(&local->config_menu, &local->mono_toggle, 11);
    menu_attach(&local->config_menu, &local->config_done_button, 11);

    local->playerone_input_button.userdata = (void*)scene;
    local->playerone_input_button.click = mainmenu_enter_playerone_input_config;

    local->playertwo_input_button.userdata = (void*)scene;
    local->playertwo_input_button.click = mainmenu_enter_playertwo_input_config;

    local->video_options_button.userdata = (void*)scene;
    local->video_options_button.click = mainmenu_enter_menu_video;

    local->config_header.disabled = 1;
    menu_select(&local->config_menu, &local->playerone_input_button);

    local->config_done_button.click = mainmenu_prev_menu;
    local->config_done_button.userdata = (void*)scene;

    // sound options
    local->sound_slider.slide = menu_sound_slide;
    local->music_slider.slide = menu_music_slide;
    textslider_bindvar(&local->sound_slider, &setting->sound.sound_vol);
    textslider_bindvar(&local->music_slider, &setting->sound.music_vol);
    textselector_bindvar(&local->mono_toggle, &setting->sound.music_mono);
    local->mono_toggle.toggle = menu_mono_toggle;

    menu_set_userdata(local);
    menu_set_free_cb(menu_configuration_free);*/
}
