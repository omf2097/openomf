#include "game/scenes/mainmenu/menu_input.h"

#include "game/menu/gui.h"

typedef struct {
    component input_config_header;
    component input_left_keyboard;
    component input_right_keyboard;
    component input_custom_keyboard;
    component input_joystick1;
    component input_joystick2;
    component input_done_button;
} input_menu_data;

void menu_input_free(menu *menu) {
    input_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->input_config_header);
    textbutton_free(&local->input_right_keyboard);
    textbutton_free(&local->input_left_keyboard);
    textbutton_free(&local->input_custom_keyboard);
    textbutton_free(&local->input_joystick1);
    textbutton_free(&local->input_joystick2);
    textbutton_free(&local->input_done_button);
    free(local);
}

void menu_input_create(menu *menu) {
    input_menu_data *local = malloc(sizeof(input_menu_data));

    textbutton_create(&local->input_config_header, &font_large, "CHOOSE INPUT DEVICE");
    local->input_config_header.disabled = 1;
    textbutton_create(&local->input_right_keyboard, &font_large, "RIGHT KEYBOARD");
    textbutton_create(&local->input_left_keyboard, &font_large, "LEFT KEYBOARD");
    textbutton_create(&local->input_custom_keyboard, &font_large, "CUSTOM KEYBOARD");
    textbutton_create(&local->input_joystick1, &font_large, "JOYSTICK 1");
    textbutton_create(&local->input_joystick2, &font_large, "JOYSTICK 2");
    textbutton_create(&local->input_done_button, &font_large, "DONE");
    menu_attach(&local->input_config_menu, &local->input_config_header, 22);
    menu_attach(&local->input_config_menu, &local->input_right_keyboard, 11);
    menu_attach(&local->input_config_menu, &local->input_left_keyboard, 11);
    menu_attach(&local->input_config_menu, &local->input_custom_keyboard, 11);
    menu_attach(&local->input_config_menu, &local->input_joystick1, 11);
    menu_attach(&local->input_config_menu, &local->input_joystick2, 33);
    menu_attach(&local->input_config_menu, &local->input_done_button, 11);

    menu_select(&local->input_config_menu, &local->input_right_keyboard);

    local->input_right_keyboard.click = mainmenu_set_right_keyboard;
    local->input_right_keyboard.userdata = (void*)scene;

    local->input_left_keyboard.click = mainmenu_set_left_keyboard;
    local->input_left_keyboard.userdata = (void*)scene;

    local->input_custom_keyboard.click = mainmenu_enter_custom_keyboard_config;
    local->input_custom_keyboard.userdata = (void*)scene;

    int jcount = joystick_count();
    local->input_joystick1.click = mainmenu_set_joystick1;
    local->input_joystick1.userdata = (void*)scene;
    if (jcount < 1) {
        local->input_joystick1.disabled = 1;
    }

    local->input_joystick2.click = mainmenu_set_joystick2;
    local->input_joystick2.userdata = (void*)scene;
    if (jcount < 2) {
        local->input_joystick2.disabled = 1;
    }

    local->input_done_button.click = mainmenu_prev_menu;
    local->input_done_button.userdata = (void*)scene;

    menu_set_userdata(local);
    menu_set_free_cb(menu_input_free);
}
