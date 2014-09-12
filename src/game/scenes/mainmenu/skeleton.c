#include "game/scenes/mainmenu/menu_configuration.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {

} configuration_menu_data;

void menu_video_free(menu *menu) {
    configuration_menu_data *local = menu_get_userdata(menu);
    free(local);
}

void menu_video_create(menu *menu) {
    configuration_menu_data *local = malloc(sizeof(configuration_menu_data));
    menu_set_userdata(local);
    menu_set_free_cb(menu_video_free);
}
