#include <stdio.h>

#include "game/scenes/mainmenu/menu_keyboard.h"
#include "game/scenes/mainmenu/menu_presskey.h"

#include "controller/keyboard.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/log.h"

static const char *keynames[] = {"JUMP UP",   "JUMP RIGHT", "WALK RIGHT", "DUCK FORWARD", "DUCK",
                                 "DUCK BACK", "WALK BACK",  "JUMP LEFT",  "PUNCH",        "KICK"};

// TODO: This menu is using gui_frame instead of component. I consider it as hack.
// The reasons are that the menu is positioned outside its parent component.
// Because of this it also needs its own background. Both of these things are
// handled by special gui_frame. The menu is also added by special function
// menu_link_menu instead of standard menu_set_submenu.

typedef struct {
    gui_frame *frame;
    component *keys[10];
    int selected_player;
} keyboard_menu_local;

void menu_keyboard_free(component *c) {
    keyboard_menu_local *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

char **menu_get_key(int player, int keynum) {
    settings_keyboard *k = &settings_get()->keys;
    switch(player) {
        case 1:
            switch(keynum) {
                case 0:
                    return &k->key1_jump_up;
                case 1:
                    return &k->key1_jump_right;
                case 2:
                    return &k->key1_walk_right;
                case 3:
                    return &k->key1_duck_forward;
                case 4:
                    return &k->key1_duck;
                case 5:
                    return &k->key1_duck_back;
                case 6:
                    return &k->key1_walk_back;
                case 7:
                    return &k->key1_jump_left;
                case 8:
                    return &k->key1_punch;
                case 9:
                    return &k->key1_kick;
            }
            break;
        case 2:
            switch(keynum) {
                case 0:
                    return &k->key2_jump_up;
                case 1:
                    return &k->key2_jump_right;
                case 2:
                    return &k->key2_walk_right;
                case 3:
                    return &k->key2_duck_forward;
                case 4:
                    return &k->key2_duck;
                case 5:
                    return &k->key2_duck_back;
                case 6:
                    return &k->key2_walk_back;
                case 7:
                    return &k->key2_jump_left;
                case 8:
                    return &k->key2_punch;
                case 9:
                    return &k->key2_kick;
            }
            break;
    }
    return NULL;
}

static void set_button_text(keyboard_menu_local *local, int i) {
    assert(i < 10);
    char tmp_buf[32];
    snprintf(tmp_buf, 32, "%-19s%12s", keynames[i], *menu_get_key(local->selected_player, i));
    button_set_text(local->keys[i], tmp_buf);
}

void menu_update_keys(component *c) {
    keyboard_menu_local *local = menu_get_userdata(c);
    for(int i = 0; i < 10; i++) {
        set_button_text(local, i);
    }
}

void menu_keyboard_done(component *c, void *userdata) {
    // Set menu as finished
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    // Apply config
    keyboard_menu_local *local = menu_get_userdata(c->parent);
    settings_keyboard *k = &settings_get()->keys;
    if(local->selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
    } else {
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
    }
    // reconfigure_controller(((scene*)userdata)->gs);
}

void menu_keyboard_set_key(component *c, void *userdata) {
    char **key = (char **)userdata;
    menu_set_submenu(c->parent, menu_presskey_create(key));
}

void menu_keyboard_keypress_done(component *c, component *submenu) {
    menu_update_keys(c);
}

gui_frame *menu_keyboard_create(scene *s, const gui_theme *theme, int selected_player) {
    keyboard_menu_local *local = omf_calloc(1, sizeof(keyboard_menu_local));
    local->selected_player = selected_player;
    local->frame = gui_frame_create(theme, 25, 5, 270, 140);
    component *menu = menu_create();
    menu_attach(menu, label_create_title("CUSTOM KEYBOARD SETUP"));
    for(int i = 0; i < 10; i++) {
        local->keys[i] = button_create("", NULL, false, false, menu_keyboard_set_key,
                                       (void *)menu_get_key(local->selected_player, i));
        set_button_text(local, i);
        menu_attach(menu, local->keys[i]);
    }
    menu_attach(menu, button_create("DONE", "Leave custom keyboard setup.", false, false, menu_keyboard_done, s));

    gui_frame_set_root(local->frame, menu);
    gui_frame_layout(local->frame);

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_keyboard_free);
    menu_set_submenu_done_cb(menu, menu_keyboard_keypress_done);
    return local->frame;
}
