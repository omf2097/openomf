#include "game/scenes/mainmenu/menu_main.h"
#include "game/scenes/mainmenu/menu_configuration.h"
#include "game/scenes/mainmenu/menu_gameplay.h"
#include "game/scenes/mainmenu/menu_help.h"
#include "game/scenes/mainmenu/menu_net.h"
#include "game/scenes/mainmenu/menu_widget_ids.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "resources/ids.h"

void mainmenu_quit(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_CREDITS);
}

void mainmenu_1v1(component *c, void *userdata) {
    scene *s = userdata;

    // Set up controllers
    settings_keyboard *k = &settings_get()->keys;
    if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 0);
    } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 0, k->joy_name1, k->joy_offset1);
    }

    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 0)),
                             settings_get()->gameplay.difficulty);
    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 1)),
                             settings_get()->gameplay.difficulty);
    _setup_ai(s->gs, 1);

    // reset match settings if the user changed config
    game_state_match_settings_reset(s->gs);

    // clear player1's name so we know to open to the pilot select
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->name[0] = '\0';

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_1v2(component *c, void *userdata) {
    scene *s = userdata;

    settings_keyboard *k = &settings_get()->keys;
    if(k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 0);
    } else if(k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 0, k->joy_name1, k->joy_offset1);
    }

    if(k->ctrl_type2 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 1);
    } else if(k->ctrl_type2 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 1, k->joy_name2, k->joy_offset2);
    }

    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 0)), AI_DIFFICULTY_CHAMPION);
    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 1)), AI_DIFFICULTY_CHAMPION);

    // reset match settings if the user changed config
    game_state_match_settings_reset(s->gs);

    // clear player1's name so we know to open to the pilot select
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->name[0] = '\0';

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_demo(component *c, void *userdata) {
    scene *s = userdata;

    // reset match settings if the user changed config
    game_state_match_settings_reset(s->gs);

    // Set up controllers
    game_state_init_demo(s->gs);
    game_state_set_next(s->gs, SCENE_VS);
}

void mainmenu_soreboard(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_SCOREBOARD);
}

void mainmenu_mechlab(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p2 = game_state_get_player(s->gs, 1);
    sd_pilot_free(p2->pilot);
    omf_free(p2->pilot);
    p2->pilot = NULL;
    game_state_set_next(s->gs, SCENE_MECHLAB);
}

void mainmenu_enter_configuration(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_configuration_create(s));
}

void mainmenu_enter_gameplay(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_gameplay_create(s));
}

void mainmenu_enter_network(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_net_create(s));
}

void mainmenu_enter_help(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_help_create(s));
}

component *menu_main_create(scene *s) {
    component *menu = menu_create();
    menu_attach(menu, button_create("ONE PLAYER GAME", NULL, false, false, mainmenu_1v1, s));
    menu_attach(menu, button_create("TWO PLAYER GAME", NULL, false, false, mainmenu_1v2, s));
    menu_attach(menu, button_create("TOURNAMENT PLAY", NULL, false, false, mainmenu_mechlab, s));
    component *net = button_create("NETWORK PLAY", NULL, false, false, mainmenu_enter_network, s);
    widget_set_id(net, NETWORK_BUTTON_ID);
    menu_attach(menu, net);
    menu_attach(menu, button_create("CONFIGURATION", NULL, false, false, mainmenu_enter_configuration, s));
    menu_attach(menu, button_create("GAMEPLAY", NULL, false, false, mainmenu_enter_gameplay, s));
    menu_attach(menu, button_create("HELP", NULL, false, false, mainmenu_enter_help, s));
    menu_attach(menu, button_create("DEMO", NULL, false, false, mainmenu_demo, s));
    menu_attach(menu, button_create("SCOREBOARD", NULL, false, false, mainmenu_soreboard, s));
    menu_attach(menu, button_create("QUIT", NULL, false, false, mainmenu_quit, s));
    return menu;
}
