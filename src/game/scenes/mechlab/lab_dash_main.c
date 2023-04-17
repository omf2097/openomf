#include <stdio.h>

#include "game/gui/gauge.h"
#include "game/gui/label.h"
#include "game/gui/pilotpic.h"
#include "game/gui/xysizer.h"
#include "game/scenes/mechlab/lab_dash_main.h"
#include "resources/ids.h"
#include "utils/log.h"
#include "video/video.h"

component *lab_dash_main_create(scene *s, dashboard_widgets *dw) {
    component *xy = xysizer_create();

    text_settings tconf_dark;
    text_defaults(&tconf_dark);
    tconf_dark.font = FONT_SMALL;
    tconf_dark.cforeground = color_create(0, 200, 0, 255);

    text_settings tconf_light;
    text_defaults(&tconf_light);
    tconf_light.font = FONT_SMALL;
    tconf_light.cforeground = color_create(50, 240, 50, 255);

    // Pilot image
    dw->photo = pilotpic_create(PIC_PLAYERS, 1);
    xysizer_attach(xy, dw->photo, 12, -1, -1, -1);

    // Texts
    dw->name = label_create(&tconf_light, "NO NAME");
    dw->rank = label_create(&tconf_dark, "RANK: 0");
    dw->wins = label_create(&tconf_dark, "WINS: 0");
    dw->losses = label_create(&tconf_dark, "LOSES: 0");
    dw->money = label_create(&tconf_dark, "MONEY: $ 0K");
    dw->tournament = label_create(&tconf_light, "NO TOURNAMENT");
    xysizer_attach(xy, dw->name, 12, 58, 200, 6);
    xysizer_attach(xy, dw->rank, 18, 64, 200, 6);
    xysizer_attach(xy, dw->wins, 18, 70, 200, 6);
    xysizer_attach(xy, dw->losses, 12, 76, 200, 6);
    xysizer_attach(xy, dw->money, 12, 82, 200, 6);
    xysizer_attach(xy, dw->tournament, 12, 88, 200, 6);

    // Bars and texts (bottom left side)
    xysizer_attach(xy, label_create(&tconf_dark, "POWER"), 12, 95, -1, -1);
    dw->power = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->power, 12, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "AGILITY"), 12, 106, -1, -1);
    dw->agility = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->agility, 12, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "ENDURANCE"), 12, 117, -1, -1);
    dw->endurance = gauge_create(GAUGE_SMALL, 25, 3);
    xysizer_attach(xy, dw->endurance, 12, 124, -1, -1);

    // Bars and texts (bottom middle)
    xysizer_attach(xy, label_create(&tconf_dark, "ARM POWER"), 125, 95, 200, 6);
    dw->arm_power = gauge_create(GAUGE_BIG, 8, 3);
    xysizer_attach(xy, dw->arm_power, 125, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "LEG POWER"), 125, 106, 200, 6);
    dw->leg_power = gauge_create(GAUGE_BIG, 8, 3);
    xysizer_attach(xy, dw->leg_power, 125, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "ARMOR"), 125, 117, 200, 6);
    dw->armor = gauge_create(GAUGE_BIG, 8, 3);
    xysizer_attach(xy, dw->armor, 125, 124, -1, -1);

    // Bars and texts (bottom right side)
    xysizer_attach(xy, label_create(&tconf_dark, "ARM SPEED"), 228, 95, 200, 6);
    dw->arm_speed = gauge_create(GAUGE_BIG, 8, 3);
    xysizer_attach(xy, dw->arm_speed, 228, 102, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "LEG SPEED"), 228, 106, 200, 6);
    dw->leg_speed = gauge_create(GAUGE_BIG, 7, 3);
    xysizer_attach(xy, dw->leg_speed, 228, 113, -1, -1);
    xysizer_attach(xy, label_create(&tconf_dark, "STUN RES"), 228, 117, 200, 6);
    dw->stun_resistance = gauge_create(GAUGE_BIG, 7, 3);
    xysizer_attach(xy, dw->stun_resistance, 228, 124, -1, -1);

    return xy;
}

void lab_dash_main_update(scene *s, dashboard_widgets *dw) {
    char tmp[64];
    game_player *p1;

    // Load the player information for player 1
    // P1 is always the one being edited in tournament dashboard
    p1 = game_state_get_player(s->gs, 0);

    // Set up variables properly
    snprintf(tmp, 64, "RANK: %d", p1->pilot.rank);
    label_set_text(dw->rank, tmp);
    snprintf(tmp, 64, "WINS: %d", p1->pilot.wins);
    label_set_text(dw->wins, tmp);
    snprintf(tmp, 64, "LOSES: %d", p1->pilot.losses);
    label_set_text(dw->losses, tmp);
    // TODO this needs for format with commas for the thousands seperator
    snprintf(tmp, 64, "MONEY: $ %dK", p1->pilot.money);
    label_set_text(dw->money, tmp);

    // Tournament and player name
    label_set_text(dw->name, p1->pilot.name);
    label_set_text(dw->tournament, p1->pilot.trn_desc);

#define SET_GAUGE_X(name) gauge_set_lit(dw->name, p1->pilot.name + 1)

    // Pilot stats
    SET_GAUGE_X(power);
    SET_GAUGE_X(agility);
    SET_GAUGE_X(endurance);

    // Har stats
    SET_GAUGE_X(arm_power);
    SET_GAUGE_X(leg_power);
    SET_GAUGE_X(armor);
    SET_GAUGE_X(arm_speed);
    SET_GAUGE_X(leg_speed);
    SET_GAUGE_X(stun_resistance);

    // Select pilot picture
    pilotpic_select(dw->photo, PIC_PLAYERS, p1->pilot.photo_id);

    // Palette
    palette *base_pal = video_get_base_palette();
    palette_set_player_color(base_pal, 0, p1->pilot.color_3, 0);
    palette_set_player_color(base_pal, 0, p1->pilot.color_2, 1);
    palette_set_player_color(base_pal, 0, p1->pilot.color_1, 2);
    video_force_pal_refresh();
}
