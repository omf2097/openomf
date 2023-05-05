#include "game/scenes/mechlab/lab_menu_main.h"
#include "formats/chr.h"
#include "game/common_defines.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
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
        controller_init(ctrl);
        sd_pilot *pilot = &enemy->pilot;
        game_player *p1 = game_state_get_player(s->gs, 0);
        game_player *p2 = game_state_get_player(s->gs, 1);
        p2->selectable = 0;
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
    mechlab_select_dashboard(s, DASHBOARD_NEW);
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

static const button_details details_list[] = {
  // CB, Text, Text align, Halign, Valigh, Pad top, Pad bottom, Pad left, Pad right, Disable by default
    {lab_menu_main_arena,          "ARENA",            TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0,  0, COM_DISABLED},
    {lab_menu_main_training_enter, "TRAINING COURSES", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 28, 0,
     COM_DISABLED                                                                                                          },
    {lab_menu_main_buy_enter,      "BUY",              TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0,  0, COM_DISABLED},
    {lab_menu_main_sell_enter,     "SELL",             TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0,  0, COM_DISABLED},
    {lab_menu_main_load,           "LOAD",             TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 14, 0, COM_ENABLED },
    {lab_menu_main_new,            "NEW",              TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 14, 0, COM_ENABLED },
    {lab_menu_main_delete,         "DELETE",           TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 14, 0, COM_DISABLED},
    {NULL,                         "SIM",              TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP,    2, 0, 0,  0, COM_DISABLED},
    {lab_menu_main_quit,           "QUIT",             TEXT_VERTICAL,   TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0,  0, COM_ENABLED },
    {lab_menu_main_tournament,     "NEW TOURNAMENT",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0,  0, COM_DISABLED},
};

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

static const spritebutton_tick_cb tick_cbs[] = {
    lab_menu_tick_arena,
    NULL, // lab_menu_tick_training,
    NULL, // lab_menu_tick_buy,
    NULL, // lab_menu_tick_sell,
    NULL, // lab_menu_tick_load,
    NULL, // lab_menu_tick_new,
    NULL, // lab_menu_tick_delete,
    NULL, // lab_menu_tick_sim,
    NULL, // lab_menu_tick_quit,
    NULL, // lab_menu_tick_tournament,
};

component *lab_menu_main_create(scene *s, bool character_loaded) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 8)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 2);
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
        bool enabled = details_list[i].enabled;
        if(details_list[i].enabled == COM_DISABLED && character_loaded == true) {
            enabled = COM_ENABLED;
        }
        component *button =
            spritebutton_create(&tconf, details_list[i].text, bsprite->data, enabled, details_list[i].cb, s);
        component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);

        spritebutton_set_focus_cb(button, focus_cbs[i]);
        spritebutton_set_tick_cb(button, tick_cbs[i]);
        component_tick(button);
        trnmenu_attach(menu, button);
    }

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
