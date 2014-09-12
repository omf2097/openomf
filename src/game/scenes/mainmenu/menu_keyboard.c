#include "game/scenes/mainmenu/menu_keyboard.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component input_custom_keyboard_header;
    component input_up_button;
    component input_down_button;
    component input_left_button;
    component input_right_button;
    component input_kick_button;
    component input_punch_button;
    component input_config_done_button;
} keyboard_menu_data;

void menu_keyboard_free(menu *menu) {
    keyboard_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->input_custom_keyboard_header);
    textbutton_free(&local->input_up_button);
    textbutton_free(&local->input_down_button);
    textbutton_free(&local->input_left_button);
    textbutton_free(&local->input_right_button);
    textbutton_free(&local->input_punch_button);
    textbutton_free(&local->input_kick_button);
    textbutton_free(&local->input_config_done_button);
    free(local);
}

void menu_keyboard_create(menu *menu) {
    keyboard_menu_data *local = malloc(sizeof(keyboard_menu_data));

    textbutton_create(&local->input_custom_keyboard_header, &font_large, "CUSTOM INPUT SETUP");
    textbutton_create(&local->input_up_button, &font_large, "UP:");
    textbutton_create(&local->input_down_button, &font_large, "DOWN:");
    textbutton_create(&local->input_left_button, &font_large, "LEFT:");
    textbutton_create(&local->input_right_button, &font_large, "RIGHT:");
    textbutton_create(&local->input_punch_button, &font_large, "PUNCH:");
    textbutton_create(&local->input_kick_button, &font_large, "KICK:");
    textbutton_create(&local->input_config_done_button, &font_large, "DONE");
    menu_attach(&local->input_custom_keyboard_menu, &local->input_custom_keyboard_header, 22);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_up_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_down_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_left_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_right_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_punch_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_kick_button, 11);
    menu_attach(&local->input_custom_keyboard_menu, &local->input_config_done_button, 11);

    local->input_config_header.disabled = 1;
    menu_select(&local->input_custom_keyboard_menu, &local->input_up_button);

    local->input_up_button.click = inputmenu_set_key;
    local->input_up_button.userdata = (void*)scene;

    local->input_down_button.click = inputmenu_set_key;
    local->input_down_button.userdata = (void*)scene;

    local->input_left_button.click = inputmenu_set_key;
    local->input_left_button.userdata = (void*)scene;

    local->input_right_button.click = inputmenu_set_key;
    local->input_right_button.userdata = (void*)scene;

    local->input_punch_button.click = inputmenu_set_key;
    local->input_punch_button.userdata = (void*)scene;

    local->input_kick_button.click = inputmenu_set_key;
    local->input_kick_button.userdata = (void*)scene;

    local->input_config_done_button.click = mainmenu_apply_custom_input_config;
    local->input_config_done_button.userdata = (void*)scene;
    
    // Allocate memory for the input key labels
    ((textbutton*)local->input_up_button.obj)->text = local->input_key_labels[0];
    ((textbutton*)local->input_down_button.obj)->text = local->input_key_labels[1];
    ((textbutton*)local->input_left_button.obj)->text = local->input_key_labels[2];
    ((textbutton*)local->input_right_button.obj)->text = local->input_key_labels[3];
    ((textbutton*)local->input_punch_button.obj)->text = local->input_key_labels[4];
    ((textbutton*)local->input_kick_button.obj)->text = local->input_key_labels[5];
    
    menu_set_userdata(local);
    menu_set_free_cb(menu_keyboard_free);
}
