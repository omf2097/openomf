#include "game/scenes/mainmenu/menu_presskey.h"

#include "game/menu/gui.h"
#include "utils/compat.h"

typedef struct {
    int wait_timeout;
    char **key;
} presskey_menu_local;

void menu_presskey_free(component *c) {
    presskey_menu_local *local = menu_get_userdata(c);
    free(local);
}

void menu_presskey_tick(component *c) {
    menu *m = sizer_get_obj(c);

    // Wait a bit before accepting key presses
    presskey_menu_local *local = menu_get_userdata(c);
    if(local->wait_timeout > 0) {
        local->wait_timeout--;
        return;
    }

    // See if a key has been pressed
    int keys = 0;
    const unsigned char *state = SDL_GetKeyboardState(&keys);
    for(int i = 0; i < keys; i++) {
        if(state[i]) {
            free(*(local->key));
            *(local->key) = strdup(SDL_GetScancodeName(i));
            m->finished = 1;
            return;
        }
    }
}

component* menu_presskey_create(char **key) {
    presskey_menu_local *local = malloc(sizeof(presskey_menu_local));
    memset(local, 0, sizeof(presskey_menu_local));
    local->wait_timeout = 20;
    local->key = key;

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "PRESS A KEY FOR"));
    menu_attach(menu, label_create(&font_large, "THIS ACTION ..."));
    menu_attach(menu, filler_create());

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_presskey_free);
    menu_set_tick_cb(menu, menu_presskey_tick);
    return menu;
}
