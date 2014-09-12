#include "game/scenes/mainmenu/menu_main.h"

#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"

typedef struct {
    component oneplayer_button;
    component twoplayer_button;
    component tourn_button;
    component config_button;
    component gameplay_button;
    component net_button;
    component help_button;
    component demo_button;
    component scoreboard_button;
    component quit_button;
    menu net_menu;
    menu config_menu;
    menu gameplay_menu;
} main_menu_data;

void menu_main_free(menu *menu) {
    main_menu_data *local = menu_get_userdata(menu);
    textbutton_free(&local->oneplayer_button);
    textbutton_free(&local->twoplayer_button);
    textbutton_free(&local->tourn_button);
    textbutton_free(&local->config_button);
    textbutton_free(&local->gameplay_button);
    textbutton_free(&local->net_button);
    textbutton_free(&local->help_button);
    textbutton_free(&local->demo_button);
    textbutton_free(&local->scoreboard_button);
    textbutton_free(&local->quit_button);
    /*
    menu_free(&local->config_menu);
    menu_free(&local->gameplay_menu);
    menu_free(&local->net_menu);
    */
    free(local);
}

void menu_main_create(menu *menu) {
    main_menu_data *local = malloc(sizeof(main_menu_data));
/*
    // Create Network play menu
    menu_create(&local->net_menu, 165, 5, 151, 119);
    menu_net_create(&local->net_menu);

    // Create configuration menu
    menu_create(&local->config_menu, 165, 5, 151, 119);
    menu_configuration_create(&local->config_menu);

    // Create gameplay menu
    menu_create(&local->gameplay_menu, 165, 5, 151, 119);
    menu_gameplay_create(&local->gameplay_menu);*/

    // Mainmenu components
    textbutton_create(&local->oneplayer_button, &font_large, "ONE PLAYER GAME");
    textbutton_create(&local->twoplayer_button, &font_large, "TWO PLAYER GAME");
    textbutton_create(&local->tourn_button, &font_large, "TOURNAMENT PLAY");
    textbutton_create(&local->net_button, &font_large, "NETWORK PLAY");
    textbutton_create(&local->config_button, &font_large, "CONFIGURATION");
    textbutton_create(&local->gameplay_button, &font_large, "GAMEPLAY");
    textbutton_create(&local->help_button, &font_large, "HELP");
    textbutton_create(&local->demo_button, &font_large, "DEMO");
    textbutton_create(&local->scoreboard_button, &font_large, "SCOREBOARD");
    textbutton_create(&local->quit_button, &font_large, "QUIT");
    menu_attach(menu, &local->oneplayer_button, 11);
    menu_attach(menu, &local->twoplayer_button, 11);
    menu_attach(menu, &local->tourn_button, 11);
    menu_attach(menu, &local->net_button, 11);
    menu_attach(menu, &local->config_button, 11);
    menu_attach(menu, &local->gameplay_button, 11);
    menu_attach(menu, &local->help_button, 11);
    menu_attach(menu, &local->demo_button, 11);
    menu_attach(menu, &local->scoreboard_button, 11);
    menu_attach(menu, &local->quit_button, 11);

    // Status
    local->tourn_button.disabled = 1;
    local->config_button.disabled = 0;
    local->gameplay_button.disabled = 0;
    local->net_button.disabled = 0;
    local->help_button.disabled = 1;
    local->demo_button.disabled = 0;
    local->scoreboard_button.disabled = 0;

/*
    // Events
    local->quit_button.userdata = (void*)scene;
    local->quit_button.click = mainmenu_quit;
    local->oneplayer_button.userdata = (void*)scene;
    local->oneplayer_button.click = mainmenu_1v1;
    local->twoplayer_button.userdata = (void*)scene;
    local->twoplayer_button.click = mainmenu_1v2;
    local->tourn_button.userdata = (void*)scene;
    local->tourn_button.click = mainmenu_tourn;
    local->config_button.userdata = (void*)scene;
    local->config_button.click = mainmenu_enter_menu_config;
    local->net_button.userdata = (void*)scene;
    local->net_button.click = mainmenu_enter_menu_net;
    local->gameplay_button.userdata = (void*)scene;
    local->gameplay_button.click = mainmenu_enter_menu_gameplay;
    local->demo_button.userdata = (void*)scene;
    local->demo_button.click = mainmenu_demo;
    local->scoreboard_button.userdata = (void*)scene;
    local->scoreboard_button.click = mainmenu_soreboard;
*/

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_main_free);
}
