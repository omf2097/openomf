#include <stdio.h>

#include "formats/error.h"
#include "game/gui/gauge.h"
#include "game/gui/label.h"
#include "game/gui/portrait.h"
#include "game/gui/spriteimage.h"
#include "game/gui/trn_menu.h"
#include "game/gui/xysizer.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/har_economy.h"
#include "game/scenes/mechlab/lab_dash_main.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "resources/languages.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "video/video.h"

bool lab_dash_main_photo_select(component *c, void *userdata) {
    trnmenu_finish(c->parent);
    return true;
}

bool lab_dash_main_photo_left(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    portrait_prev(dw->photo[0]);
    dw->pilot->photo_id = portrait_selected(dw->photo[0]);
    portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, dw->pilot->photo_id);
    palette_load_player_colors(&dw->pilot->palette, 0);
    return true;
}

bool lab_dash_main_photo_right(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    portrait_next(dw->photo[0]);
    dw->pilot->photo_id = portrait_selected(dw->photo[0]);
    portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, dw->pilot->photo_id);
    palette_load_player_colors(&dw->pilot->palette, 0);
    return true;
}

bool lab_dash_main_chr_load(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);

    sd_chr_file *oldchr = p1->chr;
    sd_chr_file *chr = ((sd_chr_file *)list_get(dw->savegames, dw->index));
    p1->chr = omf_calloc(1, sizeof(sd_chr_file));
    memcpy(p1->chr, chr, sizeof(sd_chr_file));

    assert(oldchr != NULL);
    assert(oldchr != chr);
    log_debug("Freeing previous CHR %s", oldchr->pilot.name);
    sd_chr_free(oldchr);
    omf_free(oldchr);

    p1->pilot = &p1->chr->pilot;

    if(dw->savegames) {
        iterator it;
        list_iter_begin(dw->savegames, &it);
        sd_chr_file *chr = NULL;

        int16_t i = 0;
        foreach(it, chr) {
            if(i != dw->index) {
                log_debug("Freeing CHR %s", chr->pilot.name);
                sd_chr_free(chr);
            }
            ++i;
        }

        list_free(dw->savegames);
        omf_free(dw->savegames);
    }

    trnmenu_finish(c->parent); // We refer to the components sizer
    return true;
}

bool lab_dash_main_chr_delete(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    const char *pilot_name = p1->pilot->name;
    sg_delete(pilot_name);
    trnmenu_finish(c->parent);
    return true;
}

bool lab_dash_main_chr_left(component *c, void *userdata) {
    log_debug("CHAR LEFT");
    dashboard_widgets *dw = userdata;
    dw->index--;
    if(dw->index < 0) {
        dw->index = list_size(dw->savegames) - 1;
    }
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    p1->pilot = &((sd_chr_file *)list_get(dw->savegames, dw->index))->pilot;
    mechlab_update(dw->scene);
    return true;
}

bool lab_dash_main_chr_right(component *c, void *userdata) {
    log_debug("CHAR RIGHT");
    dashboard_widgets *dw = userdata;
    dw->index++;
    if(dw->index >= (int16_t)list_size(dw->savegames)) {
        dw->index = 0;
    }
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    p1->pilot = &((sd_chr_file *)list_get(dw->savegames, dw->index))->pilot;
    mechlab_update(dw->scene);
    return true;
}

void lab_dash_main_chr_init(component *menu, component *submenu) {
    log_debug("init chr select submenu");
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    dw->savegames = sg_load_all();
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    // find the current character, if any, and exclude them
    // and set the first pilot in the list to be the loaded one
    // and call mechlab_update to draw it

    iterator it;
    list_iter_begin(dw->savegames, &it);

    sd_chr_file *chr = NULL;
    foreach(it, chr) {
        if(p1->chr && strcmp(p1->chr->pilot.name, chr->pilot.name) == 0) {
            sd_chr_free(chr);
            list_delete(dw->savegames, &it);
        }
    }
    dw->index = 0;
    chr = list_get(dw->savegames, 0);
    p1->pilot = &chr->pilot;
    if(!p1->chr) {
        mechlab_load_har(dw->scene, p1->pilot);
    }
    mechlab_update(dw->scene);
}

void lab_dash_sim_update_portraits(dashboard_widgets *dw) {
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);

    // try to center the opponent in the 5 element carousel
    int start = dw->sim_rank - 2;
    // TODO count other CHR pilots here
    int total_characters = p1->chr->pilot.enemies_ex_unranked + 1;
    // make sure we don't go off the end of the list
    while(start + 4 > total_characters) {
        start--;
    }
    if(start < 1) {
        start = 1;
    }
    char buf[50];
    for(int i = start, j = 0; j < 5 && i <= total_characters; i++) {
        if(i < 1) {
            continue;
        }
        if(i == dw->sim_rank) {
            component_set_pos_hints(dw->photo_highlight, 6 + (j * 60), -1);
        }
        snprintf(buf, sizeof(buf), "Rank\n%d", i);
        label_set_text(dw->ranks[j], buf);
        if(i == p1->pilot->rank) {
            portrait_set_from_sprite(dw->photo[j], p1->pilot->photo);
            j++;
            continue;
        }
        for(int k = 0; k < p1->chr->pilot.enemies_ex_unranked; k++) {
            if(p1->chr->enemies[k]->pilot.rank == i) {
                portrait_set_from_sprite(dw->photo[j], p1->chr->enemies[k]->pilot.photo);
                j++;
                break;
            }
        }
    }

    component_layout(dw->photo_highlight->parent, dw->photo_highlight->parent->x, dw->photo_highlight->parent->y,
                     dw->photo_highlight->parent->w, dw->photo_highlight->parent->h);
}

bool lab_dash_sim_left(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    if(dw->sim_rank > 1) {
        dw->sim_rank--;
    }
    if(dw->sim_rank == p1->pilot->rank) {
        lab_dash_sim_update(dw->scene, dw, p1->pilot);
        // cannot select yourself
        return false;
    } else {
        for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
            if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
                lab_dash_sim_update(dw->scene, dw, &p1->chr->enemies[i]->pilot);
            }
        }
    }
    return true;
}

bool lab_dash_sim_right(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    if(dw->sim_rank <= p1->chr->pilot.enemies_ex_unranked) {
        dw->sim_rank++;
    }
    if(dw->sim_rank == p1->pilot->rank) {
        lab_dash_sim_update(dw->scene, dw, p1->pilot);
        // cannot select yourself
        return false;
    } else {
        for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
            if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
                lab_dash_sim_update(dw->scene, dw, &p1->chr->enemies[i]->pilot);
            }
        }
    }
    return true;
}

void lab_dash_sim_init(component *menu, component *submenu) {
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    dw->sim_rank = p1->pilot->rank - 1;
    if(dw->sim_rank == 0) {
        dw->sim_rank = 2;
    }
    for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
        if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
            dw->pilot = &p1->chr->enemies[i]->pilot;
            break;
        }
    }
    // TODO load the other CHR files and add them as unranked opponents
    lab_dash_sim_update(dw->scene, dw, dw->pilot);
}

void lab_dash_sim_done(component *menu, component *submenu) {
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    dw->scene->gs->match_settings.sim = true;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    game_player *p2 = game_state_get_player(dw->scene->gs, 1);

    if(dw->sim_rank == p1->pilot->rank) {
        return;
    } else if(dw->sim_rank > p1->chr->pilot.enemies_ex_unranked + 1) {
        // TODO this would be other player characters for a 2 player match
        // fight our own character as a 2 player battle
        /*p2->pilot = <get the pilot from the savegame>;
        settings_keyboard *k = &settings_get()->keys;
        if(k->ctrl_type2 == CTRL_TYPE_KEYBOARD) {
            _setup_keyboard(dw->scene->gs, 1);
        } else if(k->ctrl_type2 == CTRL_TYPE_GAMEPAD) {
            _setup_joystick(dw->scene->gs, 1, k->joy_name2, k->joy_offset2);
        }
        chr_score_set_difficulty(game_player_get_score(game_state_get_player(dw->scene->gs, 0)),
                                 AI_DIFFICULTY_CHAMPION);
        chr_score_set_difficulty(game_player_get_score(game_state_get_player(dw->scene->gs, 1)),
                                 AI_DIFFICULTY_CHAMPION);*/
        return;

    } else {
        controller *ctrl = omf_calloc(1, sizeof(controller));
        controller_init(ctrl, dw->scene->gs);
        for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
            if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
                p2->pilot = &p1->chr->enemies[i]->pilot;
            }
        }
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

        ai_controller_create(ctrl, difficulty, p2->pilot, p2->pilot->pilot_id);
        chr_score_set_difficulty(game_player_get_score(game_state_get_player(dw->scene->gs, 0)), difficulty);
        game_player_set_ctrl(p2, ctrl);
    }

    // reset the score between matches in tournament mode
    // assume we used the score by now if we need it for
    // winnings calculations, etc
    chr_score_reset_wins(game_player_get_score(p1));
    chr_score_reset(game_player_get_score(p1), 1);
    // doesn't need to be selectable
    game_player_set_selectable(p2, 1);
    // set the score difficulty
    game_state_set_next(dw->scene->gs, SCENE_VS);
    return;
}

void lab_dash_main_chr_done(component *menu, component *submenu) {
    log_debug("end chr select submenu");
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    // We can get here either when the user backs out of the menu
    // or when they select something. In either case we want to makes
    // sure that if there's a player->chr set, the player->pilot matches
    // that CHR, that we do not free that CHR from the savegame list
    // and if there's no CHR we null out the pilot as well.
    iterator it;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);

    sd_chr_file *chr = NULL;

    if(p1->chr) {
        // character is loaded, revert the pilot to it
        p1->pilot = &p1->chr->pilot;
    } else if(p1->pilot) {
        // no character is loaded, we need to go back to nothing
        sd_sprite_free(dw->pilot->photo);
        omf_free(dw->pilot->photo);
        p1->pilot = NULL; // freed below
    }

    if(dw->savegames) {
        list_iter_begin(dw->savegames, &it);
        foreach(it, chr) {
            log_debug("freeing CHR %s", chr->pilot.name);
            sd_chr_free(chr);
        }
        list_free(dw->savegames);
        omf_free(dw->savegames);
    }

    mechlab_update(dw->scene);
}

component *lab_dash_main_create(scene *s, dashboard_widgets *dw) {
    component *xy = xysizer_create();

    dw->scene = s;
    game_player *p1 = game_state_get_player(s->gs, 0);
    dw->pilot = p1->pilot;

    dw->savegames = NULL;
    dw->index = 0;

    // Pilot image
    dw->photo[0] = portrait_create(PIC_PLAYERS, 0);
    if(p1->pilot->photo) {
        log_debug("loading pilot photo from pilot");
        portrait_set_from_sprite(dw->photo[0], dw->pilot->photo);
    } else {
        dw->pilot->photo = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_create(dw->pilot->photo);
        log_debug("selecting default pilot photo");
        dw->pilot->photo_id = portrait_selected(dw->photo[0]);
        portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, 0);
    }

    palette_load_player_colors(&dw->pilot->palette, 0);

    xysizer_attach(xy, dw->photo[0], 12, -1, -1, -1);

    // Texts
    dw->name = label_create("NO NAME");
    dw->rank = label_create("RANK: 0");
    dw->wins = label_create("WINS: 0");
    dw->losses = label_create("LOSES: 0");
    dw->money = label_create("MONEY: $ 0K");
    dw->tournament = label_create("NO TOURNAMENT");
    dw->har_name = label_create("HAR NAME");
    dw->har_moves = label_create("HAR MOVES");

    label_set_text_color(dw->name, MECHLAB_BRIGHT_GREEN);
    label_set_font(dw->name, FONT_SMALL);
    label_set_text_color(dw->rank, MECHLAB_DARK_GREEN);
    label_set_font(dw->rank, FONT_SMALL);
    label_set_text_color(dw->wins, MECHLAB_DARK_GREEN);
    label_set_font(dw->wins, FONT_SMALL);
    label_set_text_color(dw->losses, MECHLAB_DARK_GREEN);
    label_set_font(dw->losses, FONT_SMALL);
    label_set_text_color(dw->money, MECHLAB_DARK_GREEN);
    label_set_font(dw->money, FONT_SMALL);
    label_set_text_color(dw->tournament, MECHLAB_BRIGHT_GREEN);
    label_set_font(dw->tournament, FONT_SMALL);
    label_set_text_color(dw->har_name, MECHLAB_BRIGHT_GREEN);
    label_set_font(dw->har_name, FONT_SMALL);
    label_set_text_horizontal_align(dw->har_name, TEXT_ALIGN_CENTER);
    label_set_text_color(dw->har_moves, MECHLAB_BRIGHT_GREEN);
    label_set_font(dw->har_moves, FONT_SMALL);
    label_set_text_horizontal_align(dw->har_moves, TEXT_ALIGN_CENTER);

    xysizer_attach(xy, dw->name, 12, 58, 200, 6);
    xysizer_attach(xy, dw->rank, 18, 64, 200, 6);
    xysizer_attach(xy, dw->wins, 18, 70, 200, 6);
    xysizer_attach(xy, dw->losses, 12, 76, 200, 6);
    xysizer_attach(xy, dw->money, 12, 82, 200, 6);
    xysizer_attach(xy, dw->tournament, 12, 88, 200, 6);
    xysizer_attach(xy, dw->har_name, 220, 2, 100, 6);
    xysizer_attach(xy, dw->har_moves, 220, 19, 100, 70);

    return lab_dash_main_create_gauges(xy, dw, p1->pilot);
}

component *lab_dash_sim_create(scene *s, dashboard_widgets *dw) {
    component *xy = xysizer_create();

    dw->scene = s;
    game_player *p1 = game_state_get_player(s->gs, 0);

    // Pilot image
    dw->sim_rank = p1->pilot->rank - 1;
    if(dw->sim_rank < 1) {
        dw->sim_rank = 2;
    }
    for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
        if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
            dw->pilot = &p1->chr->enemies[i]->pilot;
            break;
        }
    }

    surface *sur = omf_calloc(sizeof(surface), 1);
    unsigned char buf[60 * 70];
    memset(buf, 0xf8, sizeof(buf));
    surface_create_from_data(sur, 60, 70, buf);

    dw->photo_highlight = spriteimage_create(sur);
    spriteimage_set_owns_sprite(dw->photo_highlight, true);

    xysizer_attach(xy, dw->photo_highlight, 6 + (2 * 60), -1, -1, -1);

    for(int i = 0; i < 5; i++) {
        dw->photo[i] = portrait_create(PIC_PLAYERS, 0);
        xysizer_attach(xy, dw->photo[i], 6 + (i * 60), -1, -1, -1);
        dw->ranks[i] = label_create("NO RANK");
        label_set_font(dw->ranks[i], FONT_SMALL);
        label_set_text_horizontal_align(dw->ranks[i], TEXT_ALIGN_CENTER);
        label_set_text_color(dw->ranks[i], TEXT_MEDIUM_GREEN);
        xysizer_attach(xy, dw->ranks[i], 6 + (i * 60), 58, 50, -1);
    }

    lab_dash_sim_update_portraits(dw);

    // Texts
    dw->name = label_create("NAME: NO NAME");
    dw->har_name = label_create("MODEL: NO MODEL");
    dw->wins = label_create("WINS: 0");
    dw->losses = label_create("LOSES: 0");

    label_set_text_color(dw->name, TEXT_DARK_GREEN);
    label_set_font(dw->name, FONT_SMALL);
    label_set_text_color(dw->har_name, TEXT_DARK_GREEN);
    label_set_font(dw->har_name, FONT_SMALL);
    label_set_text_color(dw->wins, TEXT_DARK_GREEN);
    label_set_font(dw->wins, FONT_SMALL);
    label_set_text_color(dw->losses, TEXT_DARK_GREEN);
    label_set_font(dw->losses, FONT_SMALL);

    xysizer_attach(xy, dw->name, 18, 70, 200, 6);
    xysizer_attach(xy, dw->har_name, 12, 76, 200, 6);
    xysizer_attach(xy, dw->wins, 168, 70, 200, 6);
    xysizer_attach(xy, dw->losses, 162, 76, 200, 6);

    return lab_dash_main_create_gauges(xy, dw, dw->pilot);
}

component *lab_dash_main_create_gauges(component *xy, dashboard_widgets *dw, sd_pilot *pilot) {
    component *power = label_create("POWER");
    component *agility = label_create("AGILITY");
    component *endurance = label_create("ENDURANCE");
    component *arm_power = label_create("ARM POWER");
    component *leg_power = label_create("LEG POWER");
    component *armor = label_create("ARMOR");
    component *arm_speed = label_create("ARM SPEED");
    component *leg_speed = label_create("LEG SPEED");
    component *stun_res = label_create("STUN RES");

    component *m[] = {power, agility, endurance, arm_power, leg_power, armor, arm_speed, leg_speed, stun_res};
    for(int i = 0; i < 9; i++) {
        label_set_font(m[i], FONT_SMALL);
        label_set_text_color(m[i], MECHLAB_DARK_GREEN);
    }

    // Bars and texts (bottom left side)
    xysizer_attach(xy, power, 12, 95, 200, 6);
    dw->power = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->power, 12, 102, -1, -1);
    xysizer_attach(xy, agility, 12, 106, 200, 6);
    dw->agility = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->agility, 12, 113, -1, -1);
    xysizer_attach(xy, endurance, 12, 117, 200, 6);
    dw->endurance = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->endurance, 12, 124, -1, -1);

    // Bars and texts (bottom middle)
    xysizer_attach(xy, arm_power, 125, 95, 200, 6);
    dw->arm_power = gauge_create(GAUGE_BIG, max_arm_power[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->arm_power, 125, 102, -1, -1);
    xysizer_attach(xy, leg_power, 125, 106, 200, 6);
    dw->leg_power = gauge_create(GAUGE_BIG, max_leg_power[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->leg_power, 125, 113, -1, -1);
    xysizer_attach(xy, armor, 125, 117, 200, 6);
    dw->armor = gauge_create(GAUGE_BIG, max_armor[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->armor, 125, 124, -1, -1);

    // Bars and texts (bottom right side)
    xysizer_attach(xy, arm_speed, 228, 95, 200, 6);
    dw->arm_speed = gauge_create(GAUGE_BIG, max_arm_speed[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->arm_speed, 228, 102, -1, -1);
    xysizer_attach(xy, leg_speed, 228, 106, 200, 6);
    dw->leg_speed = gauge_create(GAUGE_BIG, max_leg_speed[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->leg_speed, 228, 113, -1, -1);
    xysizer_attach(xy, stun_res, 228, 117, 200, 6);
    dw->stun_resistance = gauge_create(GAUGE_BIG, max_stun_res[pilot->har_id] + 1, 3);
    xysizer_attach(xy, dw->stun_resistance, 228, 124, -1, -1);

    return xy;
}

void lab_dash_main_update(scene *s, dashboard_widgets *dw) {
    game_player *p1;
    char tmp[64];
    char money[32];

    // Load the player information for player 1
    // P1 is always the one being edited in tournament dashboard
    p1 = game_state_get_player(s->gs, 0);

    // Set up variables properly
    if(p1->pilot->rank == 0) {
        snprintf(tmp, sizeof(tmp), "RANK: NO RANK");
    } else {
        snprintf(tmp, sizeof(tmp), "RANK: %d", p1->pilot->rank);
    }
    label_set_text(dw->rank, tmp);
    snprintf(tmp, sizeof(tmp), "WINS: %d", p1->pilot->wins);
    label_set_text(dw->wins, tmp);
    snprintf(tmp, sizeof(tmp), "LOSES: %d", p1->pilot->losses);
    label_set_text(dw->losses, tmp);
    score_format(p1->pilot->money, money, sizeof(money));
    snprintf(tmp, sizeof(tmp), "MONEY: $ %sK", money);
    label_set_text(dw->money, tmp);

    label_set_text(dw->har_name, lang_get(31 + p1->pilot->har_id));
    label_set_text(dw->har_moves, lang_get(492 + p1->pilot->har_id));

    // Tournament and player name
    label_set_text(dw->name, p1->pilot->name);
    label_set_text(dw->tournament, p1->pilot->trn_desc);

    if(p1->pilot->photo) {
        log_debug("loading pilot photo from pilot");
        portrait_set_from_sprite(dw->photo[0], p1->pilot->photo);
    } else {
        log_debug("seletng default pilot photo");
        // Select pilot picture
        portrait_select(dw->photo[0], PIC_PLAYERS, 0);
    }

    // Palette
    palette_load_player_colors(&p1->pilot->palette, 0);

    lab_dash_main_update_gauges(dw, p1->pilot);
}

void lab_dash_sim_update(scene *s, dashboard_widgets *dw, sd_pilot *pilot) {
    char tmp[64];

    snprintf(tmp, sizeof(tmp), "WINS: %d", pilot->wins);
    label_set_text(dw->wins, tmp);
    snprintf(tmp, sizeof(tmp), "LOSES: %d", pilot->losses);
    label_set_text(dw->losses, tmp);
    snprintf(tmp, sizeof(tmp), "MODEL: %s", lang_get(31 + pilot->har_id));
    label_set_text(dw->har_name, tmp);
    snprintf(tmp, sizeof(tmp), "NAME: %s", pilot->name);
    label_set_text(dw->name, tmp);

    lab_dash_sim_update_portraits(dw);

    lab_dash_main_update_gauges(dw, pilot);
}

#define SET_GAUGE_X(name) gauge_set_lit(dw->name, pilot->name + 1)

void lab_dash_main_update_gauges(dashboard_widgets *dw, sd_pilot *pilot) {
    // Pilot stats
    SET_GAUGE_X(power);
    SET_GAUGE_X(agility);
    SET_GAUGE_X(endurance);

    gauge_set_size(dw->arm_power, max_arm_power[pilot->har_id] + 1);
    gauge_set_size(dw->leg_power, max_leg_power[pilot->har_id] + 1);
    gauge_set_size(dw->armor, max_armor[pilot->har_id] + 1);
    gauge_set_size(dw->arm_speed, max_arm_speed[pilot->har_id] + 1);
    gauge_set_size(dw->leg_speed, max_leg_speed[pilot->har_id] + 1);
    gauge_set_size(dw->stun_resistance, max_stun_res[pilot->har_id] + 1);

    // Har stats
    SET_GAUGE_X(arm_power);
    SET_GAUGE_X(leg_power);
    SET_GAUGE_X(armor);
    SET_GAUGE_X(arm_speed);
    SET_GAUGE_X(leg_speed);
    SET_GAUGE_X(stun_resistance);
}
