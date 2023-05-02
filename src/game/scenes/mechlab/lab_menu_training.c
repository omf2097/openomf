#include "game/scenes/mechlab/lab_menu_training.h"
#include "game/common_defines.h"
#include "game/gui/label.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/log.h"

// I don't care anymore, sorry
static component *label1;
static component *label2;

static uint32_t prices[] = {50,   80,   120,   180,   240,   300,   450,   600,   800,   1100,   1500,   2500,
                            4000, 7000, 10000, 14000, 20000, 28000, 40000, 55000, 75000, 100000, 140000, 200000};

void lab_menu_training_power(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    uint32_t price = prices[pilot->power];
    pilot->money -= price;
    pilot->power++;
    mechlab_update(s);
}

void lab_menu_training_check_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot->power > 23) {
        component_disable(c, 1);
        return;
    }
    uint32_t price = prices[pilot->power];
    if(price > pilot->money) {
        component_disable(c, 1);
    }
}

void lab_menu_training_agility(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    uint32_t price = prices[pilot->agility];
    pilot->money -= price;
    pilot->agility++;
    mechlab_update(s);
}

void lab_menu_training_check_agility_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot->agility > 23) {
        component_disable(c, 1);
        return;
    }
    uint32_t price = prices[pilot->agility];
    if(price > pilot->money) {
        component_disable(c, 1);
    }
}

void lab_menu_training_endurance(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    uint32_t price = prices[pilot->endurance];
    pilot->money -= price;
    pilot->endurance++;
    mechlab_update(s);
}

void lab_menu_training_check_endurance_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(pilot->endurance > 23) {
        component_disable(c, 1);
        return;
    }
    uint32_t price = prices[pilot->endurance];
    if(price > pilot->money) {
        component_disable(c, 1);
    }
}

void lab_menu_training_done(component *c, void *userdata) {
    trnmenu_finish(c->parent);
}

static const button_details details_list[] = {
    {lab_menu_training_power,     "POWER",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0, 0, COM_ENABLED},
    {lab_menu_training_agility,   "AGILITY", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0, 0, COM_ENABLED},
    {lab_menu_training_endurance, "ENDUR.",  TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0, 0, COM_ENABLED},
    {lab_menu_training_done,      "DONE",    TEXT_VERTICAL,   TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
};

void lab_menu_focus_power(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        label_set_text(label1, lang_get(512));
        if(pilot->power > 23) {
            label_set_text(label2, "UNAVAILABLE");
        } else {
            char tmp[20];
            snprintf(tmp, 20, "$ %dK", prices[pilot->power]);
            label_set_text(label2, tmp);
        }
        mechlab_set_hint(s, lang_get(533));
    }
}

void lab_menu_focus_agility(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        label_set_text(label1, lang_get(513));
        if(pilot->agility > 23) {
            label_set_text(label2, "UNAVAILABLE");
        } else {
            char tmp[20];
            snprintf(tmp, 20, "$ %dK", prices[pilot->agility]);
            label_set_text(label2, tmp);
        }
        mechlab_set_hint(s, lang_get(534));
    }
}

void lab_menu_focus_endurance(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        label_set_text(label1, lang_get(514));
        if(pilot->endurance > 23) {
            label_set_text(label2, "UNAVAILABLE");
        } else {
            char tmp[20];
            snprintf(tmp, 20, "$ %dK", prices[pilot->endurance]);
            label_set_text(label2, tmp);
        }
        mechlab_set_hint(s, lang_get(535));
    }
}

void lab_menu_focus_training_done(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        label_set_text(label1, "");
        label_set_text(label2, "");
        mechlab_set_hint(s, lang_get(536));
    }
}

static const spritebutton_focus_cb focus_cbs[] = {lab_menu_focus_power, lab_menu_focus_agility,
                                                  lab_menu_focus_endurance, lab_menu_focus_training_done};

component *lab_menu_training_create(scene *s) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 9)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 1);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = color_create(0, 0, 123, 255);

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        tconf.valign = details_list[i].valign;
        tconf.halign = details_list[i].halign;
        tconf.padding.top = details_list[i].top;
        tconf.padding.bottom = details_list[i].bottom;
        tconf.padding.left = details_list[i].left;
        tconf.padding.right = details_list[i].right;
        tconf.direction = details_list[i].dir;

        sprite *bsprite = animation_get_sprite(main_buttons, i);
        component *button =
            spritebutton_create(&tconf, details_list[i].text, bsprite->data, COM_ENABLED, details_list[i].cb, s);
        component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);

        if(i == 0) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_power_price);
        } else if(i == 1) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_agility_price);
        } else if(i == 2) {
            spritebutton_set_tick_cb(button, lab_menu_training_check_endurance_price);
        }
        component_tick(button);

        spritebutton_set_focus_cb(button, focus_cbs[i]);

        trnmenu_attach(menu, button);
    }

    tconf.direction = TEXT_HORIZONTAL;
    tconf.halign = TEXT_LEFT;
    tconf.valign = TEXT_TOP;
    tconf.cforeground = color_create(0, 200, 0, 255);
    label1 = label_create(&tconf, "");
    component_set_size_hints(label1, 90, 110);
    component_set_pos_hints(label1, 200, 148);
    trnmenu_attach(menu, label1);

    tconf.cforeground = color_create(0, 255, 0, 255);
    label2 = label_create(&tconf, "");
    component_set_size_hints(label2, 90, 110);
    component_set_pos_hints(label2, 200, 186);
    trnmenu_attach(menu, label2);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
