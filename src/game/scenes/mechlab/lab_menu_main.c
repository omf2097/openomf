#include <stdio.h>

#include "formats/chr.h"
#include "game/common_defines.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/scenes/mechlab/lab_menu_main.h"
#include "game/scenes/mechlab/lab_menu_training.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

void lab_menu_main_arena(component *c, void *userdata) {
    scene *s = userdata;
    sd_chr_enemy *enemy = mechlab_next_opponent(s);
    if(enemy) {
        // make a new AI controller
        controller *ctrl = omf_calloc(1, sizeof(controller));
        controller_init(ctrl, s->gs);
        sd_pilot *pilot = &enemy->pilot;
        game_player *p1 = game_state_get_player(s->gs, 0);
        game_player *p2 = game_state_get_player(s->gs, 1);
        p2->selectable = 0;
        if(p2->pilot) {
            sd_pilot_free(p2->pilot);
            omf_free(p2->pilot);
        }
        p2->pilot = pilot;
        // there's not an exact difficulty mapping
        // for aluminum to 1p mode, but round up to
        // veteran
        int difficulty = AI_DIFFICULTY_VETERAN;
        if(p1->pilot->difficulty == 1) {
            // Iron == Champion
            difficulty = AI_DIFFICULTY_CHAMPION;
        } else if(p1->pilot->difficulty == 2) {
            // Steel == Deadly
            difficulty = AI_DIFFICULTY_DEADLY;
        } else if(p1->pilot->difficulty == 3) {
            // Heavy Metal == F.A.A.K. 2
            difficulty = AI_DIFFICULTY_ULTIMATE;
        }
        ai_controller_create(ctrl, difficulty, pilot, p2->pilot->pilot_id);
        game_player_set_ctrl(p2, ctrl);
        // reset the score between matches in tournament mode
        // assume we used the score by now if we need it for
        // winnings calculations, etc
        chr_score_reset_wins(game_player_get_score(p1));
        chr_score_reset(game_player_get_score(p1), 1);
        // set the score difficulty
        chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 0)), difficulty);
        game_state_set_next(s->gs, SCENE_VS);
    }
}

void lab_menu_main_quit(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_MENU);
}

void lab_menu_main_buy_enter(component *c, void *userdata) {
    scene *s = userdata;
    mechlab_set_selling(s, false);
    trnmenu_set_submenu(c->parent, lab_menu_customize_create(s));
}

void lab_menu_main_sell_enter(component *c, void *userdata) {
    scene *s = userdata;
    mechlab_set_selling(s, true);
    trnmenu_set_submenu(c->parent, lab_menu_customize_create(s));
}

void lab_menu_main_training_enter(component *c, void *userdata) {
    scene *s = userdata;
    trnmenu_set_submenu(c->parent, lab_menu_training_create(s));
}

void lab_menu_main_new(component *c, void *userdata) {
    scene *s = userdata;
    mechlab_select_dashboard(s, DASHBOARD_NEW_PLAYER);
}

void lab_menu_main_tournament(component *c, void *userdata) {
    scene *s = userdata;
    mechlab_select_dashboard(s, DASHBOARD_SELECT_TOURNAMENT);
    mechlab_enter_trnselect_menu(s);
}

void lab_menu_main_load(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(sg_count() == 1 && p1->chr) {
        // TODO one and only loaded
        return;
    } else if(sg_count() == 0) {
        // TODO none to load
        return;
    }
    trnmenu_set_submenu(c->parent, mechlab_chrload_menu_create(s));
}

void lab_menu_main_delete(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(sg_count() < 2 && p1->chr) {
        // none to delete
        return;
    }
    trnmenu_set_submenu(c->parent, mechlab_chrdelete_menu_create(s));
}

void lab_menu_main_sim(component *c, void *userdata) {
    scene *s = userdata;
    mechlab_select_dashboard(s, DASHBOARD_SIM);
    trnmenu_set_submenu(c->parent, mechlab_sim_menu_create(s));
}

enum lab_buttons
{
    LAB_BTN_ARENA,
    LAB_BTN_TRAININGCOURSES,
    LAB_BTN_BUY,
    LAB_BTN_SELL,
    LAB_BTN_LOAD,
    LAB_BTN_NEW,
    LAB_BTN_DELETE,
    LAB_BTN_SIM,
    LAB_BTN_QUIT,
    LAB_BTN_NEWTOURNAMENT,
};

// clang-format off
static const button_details details_list[] = {
    // CB, Text, Text align, Halign, Valigh, Pad top, Pad bottom, Pad left, Pad right, Disable by default
    {lab_menu_main_arena,          "ARENA",            TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2,  0}, true},
    {lab_menu_main_training_enter, "TRAINING COURSES", TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {22, 0, 0, 0}, true},
    {lab_menu_main_buy_enter,      "BUY",              TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2,  0}, true},
    {lab_menu_main_sell_enter,     "SELL",             TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2,  0}, true},
    {lab_menu_main_load,           "LOAD",             TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {12, 0, 0, 0}, true},
    {lab_menu_main_new,            "NEW",              TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {12, 0, 0, 0}, false},
    {lab_menu_main_delete,         "DELETE",           TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {12, 0, 0, 0}, true},
    {lab_menu_main_sim,            "SIM",              TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_TOP,    {0, 0, 2,  0}, true},
    {lab_menu_main_quit,           "QUIT",             TEXT_ROW_VERTICAL,   TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {1, 0, 0,  0}, false},
    {lab_menu_main_tournament,     "NEW TOURNAMENT",   TEXT_ROW_HORIZONTAL, TEXT_ALIGN_CENTER, TEXT_ALIGN_MIDDLE, {0, 0, 0,  0}, true},
};
// clang-format on

void lab_menu_focus_arena(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        sd_chr_enemy *enemy = mechlab_next_opponent(s);
        if(enemy) {
            char tmp[100];
            snprintf(tmp, 100, lang_get(537), enemy->pilot.name);
            mechlab_set_hint(s, tmp);
        }
    }
}

void lab_menu_focus_training(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(538));
    }
}

void lab_menu_focus_buy(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(539));
    }
}

void lab_menu_focus_sell(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(540));
    }
}

void lab_menu_focus_load(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(541));
    }
}

void lab_menu_focus_new(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(542));
    }
}

void lab_menu_focus_delete(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(543));
    }
}

void lab_menu_focus_sim(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(544));
    }
}

void lab_menu_focus_quit(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(545));
    }
}

void lab_menu_focus_tournament(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        mechlab_set_hint(s, lang_get(546));
    }
}

static const spritebutton_focus_cb focus_cbs[] = {
    lab_menu_focus_arena, lab_menu_focus_training, lab_menu_focus_buy, lab_menu_focus_sell, lab_menu_focus_load,
    lab_menu_focus_new,   lab_menu_focus_delete,   lab_menu_focus_sim, lab_menu_focus_quit, lab_menu_focus_tournament,
};

void lab_menu_tick_arena(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(p1->chr && p1->chr->pilot.rank > 1) {
        component_disable(c, 0);
        c->supports_select = true;
    } else {
        component_disable(c, 1);
        c->supports_select = false;
    }
}

void lab_menu_tick_in_tournament(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(p1->chr && p1->chr->pilot.rank != 0) {
        component_disable(c, 0);
        c->supports_select = true;
    } else {
        component_disable(c, 1);
        c->supports_select = false;
    }
}

static const spritebutton_tick_cb tick_cbs[] = {
    lab_menu_tick_arena,         // lab_menu_tick_arena,
    lab_menu_tick_in_tournament, // lab_menu_tick_training,
    lab_menu_tick_in_tournament, // lab_menu_tick_buy,
    lab_menu_tick_in_tournament, // lab_menu_tick_sell,
    NULL,                        // lab_menu_tick_load,
    NULL,                        // lab_menu_tick_new,
    lab_menu_tick_in_tournament, // lab_menu_tick_delete,
    lab_menu_tick_in_tournament, // lab_menu_tick_sim,
    NULL,                        // lab_menu_tick_quit,
    NULL,                        // lab_menu_tick_tournament,
};

component *lab_menu_main_create(scene *s, bool character_loaded) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 8)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 2);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, false);

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        sprite *button_sprite = animation_get_sprite(main_buttons, i);
        component *button = sprite_button_from_details(&details_list[i], NULL, button_sprite->data, s);
        spritebutton_set_font(button, FONT_SMALL);
        spritebutton_set_text_color(button, TEXT_TRN_BLUE);
        component_set_pos_hints(button, button_sprite->pos.x, button_sprite->pos.y);

        bool disabled = details_list[i].disabled;
        if(i == LAB_BTN_LOAD) {
            if(sg_count() > 0) {
                // there are save games to load
                disabled = false;
            }
        } else if(details_list[i].disabled == true && character_loaded == true) {
            disabled = false;
        }
        component_disable(button, disabled);

        spritebutton_set_focus_cb(button, focus_cbs[i]);
        spritebutton_set_tick_cb(button, tick_cbs[i]);
        component_tick(button);
        trnmenu_attach(menu, button);
    }

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
