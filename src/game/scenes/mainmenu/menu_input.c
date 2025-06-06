#include "game/scenes/mainmenu/menu_input.h"
#include "game/scenes/mainmenu/menu_keyboard.h"

#include "controller/joystick.h"
#include "controller/keyboard.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"

typedef struct {
    int selected_player;
} menu_input_local;

#define KEY_RESET(key, scancode)                                                                                       \
    omf_free(key);                                                                                                     \
    key = omf_strdup(SDL_GetScancodeName(scancode))

void menu_set_right_keyboard(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        KEY_RESET(k->key1_jump_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key1_jump_right, SDL_SCANCODE_PAGEUP);
        KEY_RESET(k->key1_walk_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key1_duck_forward, SDL_SCANCODE_PAGEDOWN);
        KEY_RESET(k->key1_duck, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key1_duck_back, SDL_SCANCODE_END);
        KEY_RESET(k->key1_walk_back, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key1_jump_left, SDL_SCANCODE_HOME);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        KEY_RESET(k->key2_jump_up, SDL_SCANCODE_W);
        KEY_RESET(k->key2_jump_right, SDL_SCANCODE_E);
        KEY_RESET(k->key2_walk_right, SDL_SCANCODE_D);
        KEY_RESET(k->key2_duck_forward, SDL_SCANCODE_C);
        KEY_RESET(k->key2_duck, SDL_SCANCODE_X);
        KEY_RESET(k->key2_duck_back, SDL_SCANCODE_Z);
        KEY_RESET(k->key2_walk_back, SDL_SCANCODE_A);
        KEY_RESET(k->key2_jump_left, SDL_SCANCODE_Q);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene *)userdata)->gs);
    } else if(local->selected_player == 2) {
        KEY_RESET(k->key2_jump_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key2_jump_right, SDL_SCANCODE_PAGEUP);
        KEY_RESET(k->key2_walk_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key2_duck_forward, SDL_SCANCODE_PAGEDOWN);
        KEY_RESET(k->key2_duck, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key2_duck_back, SDL_SCANCODE_END);
        KEY_RESET(k->key2_walk_back, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key2_jump_left, SDL_SCANCODE_HOME);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
        KEY_RESET(k->key1_jump_up, SDL_SCANCODE_W);
        KEY_RESET(k->key1_jump_right, SDL_SCANCODE_E);
        KEY_RESET(k->key1_walk_right, SDL_SCANCODE_D);
        KEY_RESET(k->key1_duck_forward, SDL_SCANCODE_C);
        KEY_RESET(k->key1_duck, SDL_SCANCODE_X);
        KEY_RESET(k->key1_duck_back, SDL_SCANCODE_Z);
        KEY_RESET(k->key1_walk_back, SDL_SCANCODE_A);
        KEY_RESET(k->key1_jump_left, SDL_SCANCODE_Q);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene *)userdata)->gs);
    }
}

void menu_set_left_keyboard(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        KEY_RESET(k->key1_jump_up, SDL_SCANCODE_W);
        KEY_RESET(k->key1_jump_right, SDL_SCANCODE_E);
        KEY_RESET(k->key1_walk_right, SDL_SCANCODE_D);
        KEY_RESET(k->key1_duck_forward, SDL_SCANCODE_C);
        KEY_RESET(k->key1_duck, SDL_SCANCODE_X);
        KEY_RESET(k->key1_duck_back, SDL_SCANCODE_Z);
        KEY_RESET(k->key1_walk_back, SDL_SCANCODE_A);
        KEY_RESET(k->key1_jump_left, SDL_SCANCODE_Q);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        KEY_RESET(k->key2_jump_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key2_jump_right, SDL_SCANCODE_PAGEUP);
        KEY_RESET(k->key2_walk_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key2_duck_forward, SDL_SCANCODE_PAGEDOWN);
        KEY_RESET(k->key2_duck, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key2_duck_back, SDL_SCANCODE_END);
        KEY_RESET(k->key2_walk_back, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key2_jump_left, SDL_SCANCODE_HOME);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene *)userdata)->gs);
    } else if(local->selected_player == 2) {
        KEY_RESET(k->key2_jump_up, SDL_SCANCODE_W);
        KEY_RESET(k->key2_jump_right, SDL_SCANCODE_E);
        KEY_RESET(k->key2_walk_right, SDL_SCANCODE_D);
        KEY_RESET(k->key2_duck_forward, SDL_SCANCODE_C);
        KEY_RESET(k->key2_duck, SDL_SCANCODE_X);
        KEY_RESET(k->key2_duck_back, SDL_SCANCODE_Z);
        KEY_RESET(k->key2_walk_back, SDL_SCANCODE_A);
        KEY_RESET(k->key2_jump_left, SDL_SCANCODE_Q);
        KEY_RESET(k->key2_punch, SDL_SCANCODE_LSHIFT);
        KEY_RESET(k->key2_kick, SDL_SCANCODE_LCTRL);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
        KEY_RESET(k->key1_jump_up, SDL_SCANCODE_UP);
        KEY_RESET(k->key1_jump_right, SDL_SCANCODE_PAGEUP);
        KEY_RESET(k->key1_walk_right, SDL_SCANCODE_RIGHT);
        KEY_RESET(k->key1_duck_forward, SDL_SCANCODE_PAGEDOWN);
        KEY_RESET(k->key1_duck, SDL_SCANCODE_DOWN);
        KEY_RESET(k->key1_duck_back, SDL_SCANCODE_END);
        KEY_RESET(k->key1_walk_back, SDL_SCANCODE_LEFT);
        KEY_RESET(k->key1_jump_left, SDL_SCANCODE_HOME);
        KEY_RESET(k->key1_punch, SDL_SCANCODE_RETURN);
        KEY_RESET(k->key1_kick, SDL_SCANCODE_RSHIFT);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene *)userdata)->gs);
    }
}

void menu_set_joystick1(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        omf_free(k->joy_name1);
        k->joy_name1 = omf_strdup(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset1 = joystick_offset(joystick_nth_id(1), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        omf_free(k->joy_name2);
        k->joy_name2 = omf_strdup(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset2 = joystick_offset(joystick_nth_id(1), k->joy_name2);
    }
    reconfigure_controller(((scene *)userdata)->gs);
}

void menu_set_joystick2(component *c, void *userdata) {
    menu_input_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        omf_free(k->joy_name1);
        k->joy_name1 = omf_strdup(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset1 = joystick_offset(joystick_nth_id(2), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        omf_free(k->joy_name2);
        k->joy_name2 = omf_strdup(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset2 = joystick_offset(joystick_nth_id(2), k->joy_name2);
    }
    reconfigure_controller(((scene *)userdata)->gs);
}

void menu_set_custom_keyboard(component *c, void *u) {
    scene *s = u;
    menu_input_local *local = menu_get_userdata(c->parent);
    const gui_theme *theme = component_get_theme(c);
    menu_link_menu(c->parent, menu_keyboard_create(s, theme, local->selected_player));
}

void menu_input_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_input_free(component *c) {
    menu_input_local *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

component *menu_input_create(scene *s, int player_id) {
    menu_input_local *local = omf_calloc(1, sizeof(menu_input_local));
    const char *input_delays[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    local->selected_player = player_id;

    component *menu = menu_create();

    str tmp;
    str_from_format(&tmp, "CHOOSE INPUT\nDEVICE FOR\nPLAYER %d", local->selected_player);
    menu_attach(menu, label_create_title(str_c(&tmp)));
    str_free(&tmp);

    menu_attach(menu, filler_create());
    menu_attach(
        menu, button_create("RIGHT KEYBOARD",
                            "This will use the numeric keypad for movement, enter for punch and right shift for kick.",
                            false, false, menu_set_right_keyboard, s));
    menu_attach(menu,
                button_create("LEFT KEYBOARD",
                              "This will set 'q', 'w', and 'e' for jumping directions, 'a' and 'd' for left and "
                              "right and 'z', 'x' and 'c' for ducking. Tab and ctrl control punching and kicking.",
                              false, false, menu_set_left_keyboard, s));
    menu_attach(menu, button_create("CUSTOM KEYBOARD", "Invent your own keyboard settings.", false, false,
                                    menu_set_custom_keyboard, s));
    component *joy1 = button_create("JOYSTICK 1", "Use joystick 1.", false, false, menu_set_joystick1, s);
    component *joy2 = button_create("JOYSTICK 2", "Use joystick 2.", false, false, menu_set_joystick2, s);
    int jcount = joystick_count();
    if(jcount < 1) {
        component_disable(joy1, 1);
    }
    if(jcount < 2) {
        component_disable(joy2, 1);
    }
    menu_attach(menu, joy1);
    menu_attach(menu, joy2);

    int *bind_var = &settings_get()->keys.input1_delay;

    if(player_id == 2) {
        bind_var = &settings_get()->keys.input2_delay;
    }
    menu_attach(menu, textselector_create_bind_opts("INPUT DELAY",
                                                    "Set input delay, useful for local netplay practice, default is 0",
                                                    NULL, NULL, bind_var, input_delays, 11));

    menu_attach(menu, button_create("DONE", "Leave without changing anything.", false, false, menu_input_done, NULL));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_input_free);
    return menu;
}
