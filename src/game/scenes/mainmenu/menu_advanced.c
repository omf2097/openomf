#include "game/scenes/mainmenu/menu_advanced.h"

#include "game/common_defines.h"
#include "game/gui/gui.h"
#include "game/utils/settings.h"

#include "resources/languages.h"
#include "utils/miscmath.h"

typedef struct {
    int throw_range;
    int jump_height;
    int vitality;
    int block_damage;
} menu_advanced_local;

void menu_advanced_done(component *c, void *u) {
    menu *m = sizer_get_obj(c->parent);
    menu_advanced_local *local = menu_get_userdata(c->parent);

    // convert the positions back into values
    settings_get()->advanced.throw_range = local->throw_range * 20;
    settings_get()->advanced.jump_height = (local->jump_height * 5) + 80;
    settings_get()->advanced.vitality = (local->vitality * 20) + 80;
    settings_get()->advanced.block_damage = local->block_damage * 5;

    omf_free(local);

    m->finished = 1;
}

component *menu_advanced_create(scene *s) {
    component *menu = menu_create();
    menu_advanced_local *local = omf_calloc(1, sizeof(menu_advanced_local));
    menu_set_userdata(menu, local);

    const char *on_off_opts[] = {"OFF", "ON"};
    const char *throw_ranges[] = {"0%",   "20%",  "40%",  "60%",  "80%",  "100%", "120%", "140%",
                                  "160%", "180%", "200%", "220%", "240%", "260%", "280%", "300%"};
    const char *jump_heights[] = {"80%", "85%", "90%", "95%", "100%", "105%", "110%", "115%", "120%", "125%", "130%"};
    const char *hit_pauses[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    const char *vitalities[] = {"80%",  "100%", "120%", "140%", "160%", "180%", "200%", "220%", "240%",
                                "260%", "280%", "300%", "320%", "340%", "360%", "380%", "400%"};
    const char *knock_downs[] = {"NONE", "KICKS", "PUNCHES", "BOTH"};
    const char *block_damages[] = {"0%", "5%", "10%", "15%", "20%", "25%", "30%", "35%"};

    // convert the values from config into indexes into the option lists
    local->throw_range = clamp(settings_get()->advanced.throw_range / 20, 0, 15);
    local->jump_height = clamp((settings_get()->advanced.jump_height - 80) / 5, 0, 10);
    local->vitality = clamp((settings_get()->advanced.vitality - 80) / 20, 0, 16);
    local->block_damage = clamp(settings_get()->advanced.block_damage / 5, 0, 7);
    menu_attach(menu, label_create_title("ADVANCED"));
    menu_attach(menu, filler_create());

    component *c = textselector_create_bind_opts("REHIT MODE", lang_get(276), NULL, NULL,
                                                 &settings_get()->advanced.rehit_mode, on_off_opts, 2);
    component_disable(c, 1);
    menu_attach(menu, c);

    c = textselector_create_bind_opts("DEF. THROWS", lang_get(277), NULL, NULL,
                                      &settings_get()->advanced.defensive_throws, on_off_opts, 2);

    component_disable(c, 1);
    menu_attach(menu, c);
    c = textselector_create_bind_opts("THROW RANGE", lang_get(278), NULL, NULL, &local->throw_range, throw_ranges, 16);

    menu_attach(menu, c);
    c = textselector_create_bind_opts("JUMP HEIGHT", lang_get(279), NULL, NULL, &local->jump_height, jump_heights, 11);

    menu_attach(menu, c);
    c = textselector_create_bind_opts("HIT PAUSE", lang_get(280), NULL, NULL, &settings_get()->advanced.hit_pause,
                                      hit_pauses, 11);

    menu_attach(menu, c);
    c = textselector_create_bind_opts("VITALITY x", lang_get(281), NULL, NULL, &local->vitality, vitalities, 17);

    menu_attach(menu, c);
    c = textselector_create_bind_opts("KNOCK DOWN", lang_get(282), NULL, NULL, &settings_get()->advanced.knock_down,
                                      knock_downs, 4);

    component_disable(c, 1);
    menu_attach(menu, c);
    c = textselector_create_bind_opts("BLOCK DAMAGE", lang_get(283), NULL, NULL, &local->block_damage, block_damages,
                                      8);

    component_disable(c, 1);
    menu_attach(menu, c);

    menu_attach(menu, button_create("DONE", "Go back to the gameplay menu.", false, false, menu_advanced_done, NULL));

    return menu;
}
