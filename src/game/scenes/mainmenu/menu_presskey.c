#include "game/scenes/mainmenu/menu_presskey.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"

typedef struct {
    int wait_timeout;
    int warn_timeout;
    char **key;
    component *text[2];
} presskey_menu_local;

int is_key_bound(int key) {
    settings_keyboard *k = &settings_get()->keys;
    const char *compare = SDL_GetScancodeName(key);

#define CHECK_KEY(keyname)                                                                                             \
    if(strcmp(keyname, compare) == 0) {                                                                                \
        log_debug("Key %s is already bound.", compare);                                                                \
        return 1;                                                                                                      \
    }

    CHECK_KEY(k->key1_jump_up)
    CHECK_KEY(k->key1_jump_right)
    CHECK_KEY(k->key1_walk_right)
    CHECK_KEY(k->key1_duck_forward)
    CHECK_KEY(k->key1_duck)
    CHECK_KEY(k->key1_duck_back)
    CHECK_KEY(k->key1_walk_back)
    CHECK_KEY(k->key1_jump_left)
    CHECK_KEY(k->key1_punch)
    CHECK_KEY(k->key1_kick)
    CHECK_KEY(k->key2_jump_up)
    CHECK_KEY(k->key2_jump_right)
    CHECK_KEY(k->key2_walk_right)
    CHECK_KEY(k->key2_duck_forward)
    CHECK_KEY(k->key2_duck)
    CHECK_KEY(k->key2_duck_back)
    CHECK_KEY(k->key2_walk_back)
    CHECK_KEY(k->key2_jump_left)
    CHECK_KEY(k->key2_punch)
    CHECK_KEY(k->key2_kick)

    return 0;
}

void menu_presskey_free(component *c) {
    presskey_menu_local *local = menu_get_userdata(c);
    omf_free(local);
    menu_set_userdata(c, local);
}

void menu_presskey_tick(component *c) {
    menu *m = sizer_get_obj(c);
    presskey_menu_local *local = menu_get_userdata(c);

    // Check if "key already bound" warning is enabled, and handle it.
    if(local->warn_timeout > 0) {
        local->warn_timeout--;
        if(local->warn_timeout == 0) {
            for(int i = 0; i < 2; i++) {
                label_set_text_color(local->text[i], TEXT_MEDIUM_GREEN);
            }
        }
    }

    // Wait a bit before accepting key presses
    if(local->wait_timeout > 0) {
        local->wait_timeout--;
        return;
    }

    // See if a key has been pressed
    int keys = 0;
    const unsigned char *state = SDL_GetKeyboardState(&keys);
    for(int i = 0; i < keys; i++) {
        if(i < SDL_SCANCODE_A || (i > SDL_SCANCODE_EXSEL && i < SDL_SCANCODE_KP_00) ||
           i > SDL_SCANCODE_KP_HEXADECIMAL) {
            continue;
        }

        if(state[i]) {
            if(is_key_bound(i) && strcmp(SDL_GetScancodeName(i), *(local->key)) != 0) {
                // Set texts to red as a warning
                for(int m = 0; m < 2; m++) {
                    label_set_text_color(local->text[m], 0xF6);
                }
                local->warn_timeout = 50;
                return;
            } else {
                omf_free(*(local->key));
                *(local->key) = omf_strdup(SDL_GetScancodeName(i));
                m->finished = 1;
                return;
            }
        }
    }
}

component *menu_presskey_create(char **key) {
    presskey_menu_local *local = omf_calloc(1, sizeof(presskey_menu_local));
    local->wait_timeout = 20;
    local->warn_timeout = 50;
    local->key = key;

    component *menu = menu_create();
    local->text[0] = label_create_title("PRESS A KEY FOR");
    local->text[1] = label_create_title("THIS ACTION ...");
    for(int i = 0; i < 2; i++) {
        label_set_text_color(local->text[i], TEXT_BRIGHT_GREEN);
        label_set_text_horizontal_align(local->text[i], ALIGN_TEXT_CENTER);
        menu_attach(menu, local->text[i]);
    }
    menu_attach(menu, filler_create());

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_presskey_free);
    menu_set_tick_cb(menu, menu_presskey_tick);
    return menu;
}
