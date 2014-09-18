#include <stdio.h>

#include "game/scenes/mainmenu/menu_keyboard.h"
#include "game/scenes/mainmenu/menu_presskey.h"

#include "game/menu/gui.h"
#include "game/utils/settings.h"
#include "controller/keyboard.h"
#include "utils/compat.h"
#include "utils/log.h"

static const char *keynames[] = {
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "PUNCH",
    "KICK"
};

typedef struct {
    component *keys[6];
    int selected_player;
} keyboard_menu_local;

void menu_keyboard_free(component *c) {
    keyboard_menu_local *local = menu_get_userdata(c);
    free(local);
}

char** menu_get_key(int player, int keynum) {
    settings_keyboard *k = &settings_get()->keys;
    switch(player) {
        case 1: switch(keynum) {
            case 0: return &k->key1_up;
            case 1: return &k->key1_down;
            case 2: return &k->key1_left;
            case 3: return &k->key1_right;
            case 4: return &k->key1_punch;
            case 5: return &k->key1_kick;
        }
        case 2: switch(keynum) {
            case 0: return &k->key2_up;
            case 1: return &k->key2_down;
            case 2: return &k->key2_left;
            case 3: return &k->key2_right;
            case 4: return &k->key2_punch;
            case 5: return &k->key2_kick;
        }
    }
    return NULL;
}

void menu_update_keys(component *c) {
    keyboard_menu_local *local = menu_get_userdata(c);

    char tmp_buf[32];
    for(int i = 0; i < 6; i++) {
        DEBUG("%d", local->selected_player);
        sprintf(tmp_buf, "%s: %s", keynames[i], *menu_get_key(local->selected_player, i));
        textbutton_set_text(local->keys[i], tmp_buf);
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
    reconfigure_controller(((scene*)userdata)->gs);
}

void menu_keyboard_set_key(component *c, void *userdata) {
    char **key = (char**)userdata;
    menu_set_submenu(c->parent, menu_presskey_create(key));
}

void menu_keyboard_keypress_done(component *c, component *submenu) {
    menu_update_keys(c);
}

component* menu_keyboard_create(scene *s, int selected_player) {
    keyboard_menu_local *local = malloc(sizeof(keyboard_menu_local));
    memset(local, 0, sizeof(keyboard_menu_local));
    local->selected_player = selected_player;

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "CUSTOM INPUT SETUP"));
    menu_attach(menu, filler_create());
    for(int i = 0; i < 6; i++) {
        local->keys[i] = textbutton_create(&font_large, "", COM_ENABLED, menu_keyboard_set_key, (void*)menu_get_key(local->selected_player, i));
        menu_attach(menu, local->keys[i]);
    }
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_keyboard_done, s));

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_keyboard_free);
    menu_set_submenu_done_cb(menu, menu_keyboard_keypress_done);

    menu_update_keys(menu);
    return menu;
}
