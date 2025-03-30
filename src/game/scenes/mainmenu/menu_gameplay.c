#include "game/scenes/mainmenu/menu_gameplay.h"
#include "game/scenes/mainmenu/menu_advanced.h"

#include "game/common_defines.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"

void menu_gameplay_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_gameplay_speed_slide(component *c, void *userdata, int pos) {
    scene *sc = userdata;
    game_state_set_speed(sc->gs, pos + 5);
}

void menu_enter_advanced(component *c, void *userdata) {
    scene *s = userdata;
    menu_set_submenu(c->parent, menu_advanced_create(s));
}

component *menu_gameplay_create(scene *s) {
    const char *fightmode_opts[] = {"NORMAL", "HYPER"};
    const char *hazard_opts[] = {"OFF", "ON"};

    component *menu = menu_create();
    menu_attach(menu, label_create_title("GAMEPLAY"));
    menu_attach(menu, filler_create());
    menu_attach(menu,
                textslider_create_bind("SPEED", "Change the overall speed of the game. Press left and right to change.",
                                       10, false, menu_gameplay_speed_slide, s, &settings_get()->gameplay.speed));
    menu_attach(menu, textselector_create_bind_opts(
                          "FIGHT MODE",
                          "Fight mode can be either normal or hyper. Hyper mode will enhance your special moves. Check "
                          "the robot description section of help for more information.",
                          NULL, NULL, &settings_get()->gameplay.fight_mode, fightmode_opts, 2));
    menu_attach(menu, textslider_create_bind("POWER 1",
                                             "Change the power of player 1's hits and throws. This setting will take "
                                             "effect only in two player games. Press left and right to change.",
                                             7, 0, NULL, NULL, &settings_get()->gameplay.power1));
    menu_attach(menu, textslider_create_bind("POWER 2",
                                             "Change the power of player 1's hits and throws. This setting will take "
                                             "effect only in two player games. Press left and right to change.",
                                             7, 0, NULL, NULL, &settings_get()->gameplay.power2));
    menu_attach(menu, textselector_create_bind_opts("HAZARDS",
                                                    "Some arenas have dangerous environments: spikes, electrivity, "
                                                    "fighter planes, and the like. This option turns them on and off.",
                                                    NULL, NULL, &settings_get()->gameplay.hazards_on, hazard_opts, 2));
    menu_attach(menu,
                textselector_create_bind_opts("CPU:",
                                              "This determines how well the computer fights in a one player game. This "
                                              "has no effect on two player games. Press left and right to change.",
                                              NULL, NULL, &settings_get()->gameplay.difficulty, ai_difficulty_names,
                                              NUMBER_OF_AI_DIFFICULTY_TYPES));
    menu_attach(menu, textselector_create_bind_opts("",
                                                    "This will set matches so they are one round, best two out of "
                                                    "three rounds, or best three out of five rounds.",
                                                    NULL, NULL, &settings_get()->gameplay.rounds, round_type_names,
                                                    NUMBER_OF_ROUND_TYPES));

    menu_attach(menu, button_create("ADVANCED", "Do I really have to tell you what this is?", false, false,
                                    menu_enter_advanced, s));
    menu_attach(menu, button_create("DONE", "Go back to the main menu.", false, false, menu_gameplay_done, NULL));
    return menu;
}
