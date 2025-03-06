#include <stdio.h>

#include "formats/error.h"
#include "game/gui/gauge.h"
#include "game/gui/label.h"
#include "game/gui/portrait.h"
#include "game/gui/trn_menu.h"
#include "game/gui/xysizer.h"
#include "game/scenes/mechlab.h"
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

// TODO put these somewhere central
static uint8_t max_arm_speed[11] = {7, 9, 5, 7, 10, 8, 9, 7, 10, 7, 8};
static uint8_t max_leg_speed[11] = {9, 10, 6, 7, 9, 9, 8, 7, 8, 6, 7};
static uint8_t max_arm_power[11] = {6, 6, 10, 9, 5, 7, 7, 6, 6, 7, 8};
static uint8_t max_leg_power[11] = {7, 7, 9, 5, 6, 8, 6, 8, 7, 8, 8};
static uint8_t max_stun_resistance[11] = {9, 7, 9, 7, 8, 7, 7, 8, 8, 8, 7};
static uint8_t max_armor[11] = {6, 8, 8, 9, 7, 9, 7, 10, 7, 7, 8};

void lab_dash_main_photo_select(component *c, void *userdata) {
    trnmenu_finish(c->parent);
}

void lab_dash_main_photo_left(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    portrait_prev(dw->photo);
    dw->pilot->photo_id = portrait_selected(dw->photo);
    portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, dw->pilot->photo_id);
    palette_load_player_colors(&dw->pilot->palette, 0);
}

void lab_dash_main_photo_right(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    portrait_next(dw->photo);
    dw->pilot->photo_id = portrait_selected(dw->photo);
    portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, dw->pilot->photo_id);
    palette_load_player_colors(&dw->pilot->palette, 0);
}

void lab_dash_main_chr_load(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);

    sd_chr_file *oldchr = p1->chr;
    sd_chr_file *chr = ((sd_chr_file *)list_get(dw->savegames, dw->index));
    p1->chr = omf_calloc(1, sizeof(sd_chr_file));
    if(sg_load(p1->chr, chr->pilot.name) != SD_SUCCESS) {
        // bad save, revert to the loaded character
        omf_free(p1->chr);
        p1->chr = oldchr;
        trnmenu_finish(c->parent); // We refer to the components sizer
        return;
    }
    if(oldchr) {
        log_debug("freeing loaded CHR %s", oldchr->pilot.name);
        sd_chr_free(oldchr);
        omf_free(oldchr);
    }

    p1->pilot = &p1->chr->pilot;

    if(dw->savegames) {
        iterator it;
        list_iter_begin(dw->savegames, &it);
        sd_chr_file *chr = NULL;

        foreach(it, chr) {
            log_debug("freeing CHR %s", chr->pilot.name);
            sd_chr_free(chr);
        }

        list_free(dw->savegames);
        omf_free(dw->savegames);
    }

    omf_free(settings_get()->tournament.last_name);
    settings_get()->tournament.last_name = omf_strdup(p1->pilot->name);
    settings_save();
    trnmenu_finish(c->parent); // We refer to the components sizer
}

void lab_dash_main_chr_delete(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    const char *pilot_name = p1->pilot->name;
    sg_delete(pilot_name);
    trnmenu_finish(c);
}

void lab_dash_main_chr_left(component *c, void *userdata) {
    log_debug("CHAR LEFT");
    dashboard_widgets *dw = userdata;
    dw->index--;
    if(dw->index < 0) {
        dw->index = list_size(dw->savegames) - 1;
    }
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    p1->pilot = &((sd_chr_file *)list_get(dw->savegames, dw->index))->pilot;
    mechlab_update(dw->scene);
}

void lab_dash_main_chr_right(component *c, void *userdata) {
    log_debug("CHAR RIGHT");
    dashboard_widgets *dw = userdata;
    dw->index++;
    if(dw->index >= (int16_t)list_size(dw->savegames)) {
        dw->index = 0;
    }
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    p1->pilot = &((sd_chr_file *)list_get(dw->savegames, dw->index))->pilot;
    mechlab_update(dw->scene);
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

void lab_dash_sim_left(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    if(dw->sim_rank > 1) {
        dw->sim_rank--;
    }
    for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
        if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
            lab_dash_sim_update(dw->scene, dw, &p1->chr->enemies[i]->pilot);
        }
    }
}

void lab_dash_sim_right(component *c, void *userdata) {
    dashboard_widgets *dw = userdata;
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    if(dw->sim_rank < p1->chr->pilot.enemies_ex_unranked) {
        dw->sim_rank++;
    }
    for(int i = 0; i < p1->chr->pilot.enemies_ex_unranked; i++) {
        if(p1->chr->enemies[i]->pilot.rank == dw->sim_rank) {
            lab_dash_sim_update(dw->scene, dw, &p1->chr->enemies[i]->pilot);
        }
    }
}

void lab_dash_sim_init(component *menu, component *submenu) {
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    game_player *p1 = game_state_get_player(dw->scene->gs, 0);
    dw->sim_rank = p1->pilot->rank;
}

void lab_dash_sim_done(component *menu, component *submenu) {
    dashboard_widgets *dw = trnmenu_get_userdata(submenu);
    mechlab_select_dashboard(dw->scene, DASHBOARD_STATS);
    mechlab_update(dw->scene);
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

    text_settings tconf_dark;
    text_defaults(&tconf_dark);
    tconf_dark.font = FONT_SMALL;
    tconf_dark.cforeground = MECHLAB_DARK_GREEN;

    text_settings tconf_light;
    text_defaults(&tconf_light);
    tconf_light.font = FONT_SMALL;
    tconf_light.cforeground = MECHLAB_BRIGHT_GREEN;

    text_settings tconf_light_centered;
    text_defaults(&tconf_light_centered);
    tconf_light_centered.font = FONT_SMALL;
    tconf_light_centered.halign = TEXT_CENTER;
    tconf_light_centered.cforeground = MECHLAB_BRIGHT_GREEN;

    // Pilot image
    dw->photo = portrait_create(PIC_PLAYERS, 0);
    if(p1->pilot->photo) {
        log_debug("loading pilot photo from pilot");
        portrait_set_from_sprite(dw->photo, dw->pilot->photo);
    } else {
        dw->pilot->photo = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_create(dw->pilot->photo);
        log_debug("seletng default pilot photo");
        dw->pilot->photo_id = portrait_selected(dw->photo);
        portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, 0);
    }
    palette_load_player_colors(&dw->pilot->palette, 0);

    xysizer_attach(xy, dw->photo, 12, -1, -1, -1);

    // Texts
    dw->name = label_create(&tconf_light, "NO NAME");
    dw->rank = label_create(&tconf_dark, "RANK: 0");
    dw->wins = label_create(&tconf_dark, "WINS: 0");
    dw->losses = label_create(&tconf_dark, "LOSES: 0");
    dw->money = label_create(&tconf_dark, "MONEY: $ 0K");
    dw->tournament = label_create(&tconf_light, "NO TOURNAMENT");
    dw->har_name = label_create(&tconf_light_centered, "HAR NAME");
    dw->har_moves = label_create(&tconf_light_centered, "HAR MOVES");
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
    dw->pilot = p1->pilot;

    text_settings tconf_dark;
    text_defaults(&tconf_dark);
    tconf_dark.font = FONT_SMALL;
    tconf_dark.cforeground = TEXT_DARK_GREEN;

    text_settings tconf_light;
    text_defaults(&tconf_light);
    tconf_light.font = FONT_SMALL;
    tconf_light.cforeground = TEXT_MEDIUM_GREEN;

    text_settings tconf_light_centered;
    text_defaults(&tconf_light_centered);
    tconf_light_centered.font = FONT_SMALL;
    tconf_light_centered.halign = TEXT_CENTER;
    tconf_light_centered.cforeground = TEXT_MEDIUM_GREEN;

    // Pilot image
    dw->photo = portrait_create(PIC_PLAYERS, 0);
    if(p1->pilot->photo) {
        log_debug("loading pilot photo from pilot");
        portrait_set_from_sprite(dw->photo, dw->pilot->photo);
    } else {
        dw->pilot->photo = omf_calloc(1, sizeof(sd_sprite));
        sd_sprite_create(dw->pilot->photo);
        log_debug("seletng default pilot photo");
        dw->pilot->photo_id = portrait_selected(dw->photo);
        portrait_load(dw->pilot->photo, &dw->pilot->palette, PIC_PLAYERS, 0);
    }
    palette_load_player_colors(&dw->pilot->palette, 0);

    xysizer_attach(xy, dw->photo, 12, -1, -1, -1);

    // Texts
    dw->name = label_create(&tconf_dark, "NAME: NO NAME");
    dw->har_name = label_create(&tconf_dark, "MODEL: NO MODEL");
    dw->wins = label_create(&tconf_dark, "WINS: 0");
    dw->losses = label_create(&tconf_dark, "LOSES: 0");

    xysizer_attach(xy, dw->name, 18, 70, 200, 6);
    xysizer_attach(xy, dw->har_name, 12, 76, 200, 6);

    xysizer_attach(xy, dw->wins, 168, 70, 200, 6);
    xysizer_attach(xy, dw->losses, 162, 76, 200, 6);

    return lab_dash_main_create_gauges(xy, dw, p1->pilot);
}

component *lab_dash_main_create_gauges(component *xy, dashboard_widgets *dw, sd_pilot *pilot) {
    text_settings tconf_dark;
    text_defaults(&tconf_dark);
    tconf_dark.font = FONT_SMALL;
    tconf_dark.cforeground = MECHLAB_DARK_GREEN;

    // Bars and texts (bottom left side)
    xysizer_attach(xy, label_create(&tconf_dark, "POWER"), 12, 95, 200, 6);
    dw->power = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->power, 12, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "AGILITY"), 12, 106, 200, 6);
    dw->agility = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->agility, 12, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "ENDURANCE"), 12, 117, 200, 6);
    dw->endurance = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->endurance, 12, 124, -1, -1);

    // Bars and texts (bottom middle)
    xysizer_attach(xy, label_create(&tconf_dark, "ARM POWER"), 125, 95, 200, 6);
    dw->arm_power = gauge_create(GAUGE_BIG, max_arm_power[pilot->har_id], 3);
    xysizer_attach(xy, dw->arm_power, 125, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "LEG POWER"), 125, 106, 200, 6);
    dw->leg_power = gauge_create(GAUGE_BIG, max_leg_power[pilot->har_id], 3);
    xysizer_attach(xy, dw->leg_power, 125, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "ARMOR"), 125, 117, 200, 6);
    dw->armor = gauge_create(GAUGE_BIG, max_armor[pilot->har_id], 3);
    xysizer_attach(xy, dw->armor, 125, 124, -1, -1);

    // Bars and texts (bottom right side)
    xysizer_attach(xy, label_create(&tconf_dark, "ARM SPEED"), 228, 95, 200, 6);
    dw->arm_speed = gauge_create(GAUGE_BIG, max_arm_speed[pilot->har_id], 3);
    xysizer_attach(xy, dw->arm_speed, 228, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "LEG SPEED"), 228, 106, 200, 6);
    dw->leg_speed = gauge_create(GAUGE_BIG, max_leg_speed[pilot->har_id], 3);
    xysizer_attach(xy, dw->leg_speed, 228, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "STUN RES"), 228, 117, 200, 6);
    dw->stun_resistance = gauge_create(GAUGE_BIG, max_stun_resistance[pilot->har_id], 3);
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

    lab_dash_main_update_gauges(dw, pilot);
}

#define SET_GAUGE_X(name) gauge_set_lit(dw->name, pilot->name + 1)

void lab_dash_main_update_gauges(dashboard_widgets *dw, sd_pilot *pilot) {
    // Pilot stats
    SET_GAUGE_X(power);
    SET_GAUGE_X(agility);
    SET_GAUGE_X(endurance);

    gauge_set_size(dw->arm_power, max_arm_power[pilot->har_id]);
    gauge_set_size(dw->leg_power, max_leg_power[pilot->har_id]);
    gauge_set_size(dw->armor, max_armor[pilot->har_id]);
    gauge_set_size(dw->arm_speed, max_arm_speed[pilot->har_id]);
    gauge_set_size(dw->leg_speed, max_leg_speed[pilot->har_id]);
    gauge_set_size(dw->stun_resistance, max_stun_resistance[pilot->har_id]);

    // Har stats
    SET_GAUGE_X(arm_power);
    SET_GAUGE_X(leg_power);
    SET_GAUGE_X(armor);
    SET_GAUGE_X(arm_speed);
    SET_GAUGE_X(leg_speed);
    SET_GAUGE_X(stun_resistance);

    if(pilot->photo) {
        log_debug("loading pilot photo from pilot");
        portrait_set_from_sprite(dw->photo, pilot->photo);
    } else {
        log_debug("seletng default pilot photo");
        // Select pilot picture
        portrait_select(dw->photo, PIC_PLAYERS, 0);
    }

    // Palette
    palette_load_player_colors(&pilot->palette, 0);
}
