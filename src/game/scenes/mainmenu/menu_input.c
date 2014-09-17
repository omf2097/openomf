#include "game/scenes/mainmenu/menu_input.h"

#include "game/menu/gui.h"
#include "game/utils/settings.h"
#include "controller/joystick.h"
#include "controller/keyboard.h"
#include "utils/compat.h"

typedef struct {
    int selected_player;
} menu_input_local;

#define KEY_RESET(key,scancode) \
    free(key); \
    key = strdup(SDL_GetScancodeName(scancode))

void mainmenu_set_right_keyboard(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        KEY_RESET(k->key1_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key1_down, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key1_left, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key1_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene*)userdata)->gs);
    } else if(local->selected_player == 2) {
        KEY_RESET(k->key2_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key2_down, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key2_left, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key2_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
    }
}

void mainmenu_set_left_keyboard(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        KEY_RESET(k->key1_up, SDL_SCANCODE_W);
        KEY_RESET(k->key1_down, SDL_SCANCODE_S);
        KEY_RESET(k->key1_left, SDL_SCANCODE_A);
        KEY_RESET(k->key1_right, SDL_SCANCODE_D);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene*)userdata)->gs);
    } else if(local->selected_player == 2) {
        KEY_RESET(k->key2_up, SDL_SCANCODE_W);
        KEY_RESET(k->key2_down, SDL_SCANCODE_S);
        KEY_RESET(k->key2_left, SDL_SCANCODE_A);
        KEY_RESET(k->key2_right, SDL_SCANCODE_D);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
    }
}

void mainmenu_set_joystick1(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name1);
        k->joy_name1 = strdup(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset1 = joystick_offset(joystick_nth_id(1), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name2);
        k->joy_name2 = strdup(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset2 = joystick_offset(joystick_nth_id(1), k->joy_name2);
    }
    reconfigure_controller(((scene*)userdata)->gs);
}

void mainmenu_set_joystick2(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name1);
        k->joy_name1 = strdup(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset1= joystick_offset(joystick_nth_id(2), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name2);
        k->joy_name2 = strdup(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset2= joystick_offset(joystick_nth_id(2), k->joy_name2);
    }
    reconfigure_controller(((scene*)userdata)->gs);
}

void menu_input_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_input_free(component *c) {
    menu_input_local *local = menu_get_userdata(c);
    free(local);
}

component* menu_input_create(scene *s, int player_id) {
    menu_input_local *local = malloc(sizeof(menu_input_local));
    memset(local, 0, sizeof(menu_input_local));
    local->selected_player = player_id;

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "PICK INPUT DEVICE"));
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&font_large, "RIGHT KEYBOARD", COM_ENABLED, mainmenu_set_right_keyboard, s));
    menu_attach(menu, textbutton_create(&font_large, "LEFT KEYBOARD", COM_ENABLED, mainmenu_set_left_keyboard, s));
    menu_attach(menu, textbutton_create(&font_large, "CUSTOM KEYBOARD", COM_ENABLED, NULL, NULL));
    component *joy1 = textbutton_create(&font_large, "JOYSTICK 1", COM_ENABLED, mainmenu_set_joystick1, s);
    component *joy2 = textbutton_create(&font_large, "JOYSTICK 2", COM_ENABLED, mainmenu_set_joystick2, s);
    int jcount = joystick_count();
    if(jcount < 1) {
        component_disable(joy1, 1);
    }
    if(jcount < 2) {
        component_disable(joy2, 1);
    }
    menu_attach(menu, joy1);
    menu_attach(menu, joy2);
    menu_attach(menu, filler_create());
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_input_done, NULL));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_input_free);
    return menu;
}
