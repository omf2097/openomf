#include "game/scenes/mainmenu/menu_presskey.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component input_presskey_header;
} presskey_menu_data;

void menu_presskey_free(menu *menu) {
    presskey_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->input_presskey_header);
    free(local);
}

void menu_presskey_create(menu *menu) {
    presskey_menu_data *local = malloc(sizeof(presskey_menu_data));

    textbutton_create(&local->input_presskey_header, &font_large, "PRESS A KEY FOR THIS ACTION...");
    menu_attach(&local->input_presskey_menu, &local->input_presskey_header, 11);

    menu_set_userdata(local);
    menu_set_free_cb(menu_presskey_free);
}
