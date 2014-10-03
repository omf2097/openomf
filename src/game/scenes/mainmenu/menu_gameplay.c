#include "game/scenes/mainmenu/menu_gameplay.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "game/common_defines.h"

void menu_gameplay_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;
}

void menu_gameplay_speed_slide(component *c, void *userdata, int pos) {
    scene *sc = userdata;
    game_state_set_speed(sc->gs, pos);
}

component* menu_gameplay_create(scene *s) {
    const char* fightmode_opts[] = {"NORMAL","HYPER"};
    const char* hazard_opts[] = {"OFF","ON"};
    component* menu = menu_create(11);
    menu_attach(menu, label_create(&font_large, "GAMEPLAY"));
    menu_attach(menu, filler_create());
    menu_attach(menu, textslider_create_bind(&font_large, "SPEED", 10, 1, menu_gameplay_speed_slide, s, &settings_get()->gameplay.speed));
    menu_attach(menu, textselector_create_bind_opts(&font_large, "FIGHT MODE", NULL, NULL, &settings_get()->gameplay.fight_mode, fightmode_opts, 2));
    menu_attach(menu, textslider_create_bind(&font_large, "POWER 1", 8, 0, NULL, NULL, &settings_get()->gameplay.power1));
    menu_attach(menu, textslider_create_bind(&font_large, "POWER 2", 8, 0, NULL, NULL, &settings_get()->gameplay.power2));
    menu_attach(menu, textselector_create_bind_opts(&font_large, "HAZARDS", NULL, NULL, &settings_get()->gameplay.hazards_on, hazard_opts, 2));
    menu_attach(menu, textselector_create_bind_opts(&font_large, "CPU:", NULL, NULL, &settings_get()->gameplay.difficulty, ai_difficulty_names, NUMBER_OF_AI_DIFFICULTY_TYPES));
    menu_attach(menu, textselector_create_bind_opts(&font_large, "", NULL, NULL, &settings_get()->gameplay.rounds, round_type_names, NUMBER_OF_ROUND_TYPES));
    menu_attach(menu, textbutton_create(&font_large, "DONE", COM_ENABLED, menu_gameplay_done, NULL));
    return menu;
}
