#include <stdio.h>

#include "formats/pilot.h"
#include "game/gui/label.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/scenes/mechlab/lab_menu_trade.h"
#include "game/utils/formatting.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/c_array_util.h"
#include "utils/log.h"

// TODO put these somewhere central
static uint8_t max_arm_speed[11] = {6, 8, 4, 6, 9, 7, 8, 6, 9, 6, 7};
static uint8_t max_leg_speed[11] = {8, 9, 5, 6, 8, 8, 7, 6, 7, 5, 6};
static uint8_t max_arm_power[11] = {5, 5, 9, 8, 4, 6, 6, 5, 5, 6, 7};
static uint8_t max_leg_power[11] = {6, 6, 8, 4, 5, 7, 5, 7, 6, 7, 7};
static uint8_t max_stun_res[11]  = {8, 6, 8, 6, 7, 6, 6, 7, 7, 7, 6};
static uint8_t max_armor[11]     = {5, 7, 7, 8, 6, 8, 6, 9, 6, 6, 7};

static component *header_label;
static component *details_label;

static int32_t har_prices[11] = {20000, 36000, 26000, 28000, 29000, 32000, 25000, 30000, 24000, 22000, 75000};
static int32_t har_upgrade_price[11] = { 380, 400, 350, 400, 500, 330, 420, 370, 450, 360, 700 };
static int32_t upgrade_level_mutiplier[10] = { 0, 1, 3, 7, 12, 18, 30, 50, 75, 120 };

static int32_t arm_leg_multiplier = 2;
static int32_t stun_res_multiplier = 3;
static int32_t armor_multiplier = 5;

static void lab_menu_focus_arm_power(component *c, bool focused, void *userdata);
static void lab_menu_focus_arm_speed(component *c, bool focused, void *userdata);
static void lab_menu_focus_leg_power(component *c, bool focused, void *userdata);
static void lab_menu_focus_leg_speed(component *c, bool focused, void *userdata);
static void lab_menu_focus_armor(component *c, bool focused, void *userdata);
static void lab_menu_focus_stun_resistance(component *c, bool focused, void *userdata);

int calculate_trade_value(sd_pilot *pilot) {
    int trade_value = har_prices[pilot->har_id];
    for(int i = 1; i < pilot->arm_power; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->leg_power; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->arm_speed; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->leg_speed; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * arm_leg_multiplier;
    }

    for(int i = 1; i < pilot->armor; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * armor_multiplier;
    }

    for(int i = 1; i < pilot->stun_resistance; i++) {
        trade_value += har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[i] * stun_res_multiplier;
    }

    return trade_value * 0.85;
}

int har_price(int har_id) {
    return har_prices[har_id];
}

void lab_menu_customize_done(component *c, void *userdata) {
    trnmenu_finish(c->parent);
}

void lab_menu_customize_color_main(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot_set_player_color(&p1->chr->pilot, SECONDARY, (p1->chr->pilot.color_2 + 1) % 16);
    mechlab_update(s);
}

void lab_menu_customize_color_secondary(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot_set_player_color(&p1->chr->pilot, TERTIARY, (p1->chr->pilot.color_1 + 1) % 16);
    mechlab_update(s);
}

void lab_menu_customize_color_third(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot_set_player_color(&p1->chr->pilot, PRIMARY, (p1->chr->pilot.color_3 + 1) % 16);
    mechlab_update(s);
}

void lab_menu_customize_arm_power(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power] * arm_leg_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->arm_power--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power + 1] * arm_leg_multiplier;
        pilot->money -= price;
        pilot->arm_power++;
        mechlab_update(s);
    }
    lab_menu_focus_arm_power(c, true, userdata);
}

void lab_menu_customize_check_arm_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power] * arm_leg_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power + 1] * arm_leg_multiplier;
        if(price < 0 || price > pilot->money || pilot->arm_power + 1 > max_arm_power[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_leg_power(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power] * arm_leg_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->leg_power--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power + 1] * arm_leg_multiplier;
        pilot->money -= price;
        pilot->leg_power++;
        mechlab_update(s);
    }
    lab_menu_focus_leg_power(c, true, userdata);
}

void lab_menu_customize_check_leg_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power] * arm_leg_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power + 1] * arm_leg_multiplier;
        if(price < 0 || price > pilot->money || pilot->leg_power + 1 > max_leg_power[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_arm_speed(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed] * arm_leg_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->arm_speed--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed + 1] * arm_leg_multiplier;
        pilot->money -= price;
        pilot->arm_speed++;
        mechlab_update(s);
    }
    lab_menu_focus_arm_speed(c, true, userdata);
}

void lab_menu_customize_check_arm_speed_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed] * arm_leg_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed + 1] * arm_leg_multiplier;
        if(price < 0 || price > pilot->money || pilot->arm_speed + 1 > max_arm_speed[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_leg_speed(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed] * arm_leg_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->leg_speed--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed + 1] * arm_leg_multiplier;
        pilot->money -= price;
        pilot->leg_speed++;
        mechlab_update(s);
    }
    lab_menu_focus_leg_speed(c, true, userdata);
}

void lab_menu_customize_check_leg_speed_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed] * arm_leg_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed + 1] * arm_leg_multiplier;
        if(price < 0 || price > pilot->money || pilot->leg_speed + 1 > max_leg_speed[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_armor(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor] * armor_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->armor--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor + 1] * armor_multiplier;
        pilot->money -= price;
        pilot->armor++;
        mechlab_update(s);
    }
    lab_menu_focus_armor(c, true, userdata);
}

void lab_menu_customize_check_armor_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor] * armor_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor + 1] * armor_multiplier;
        if(price < 0 || price > pilot->money || pilot->armor + 1 > max_armor[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_stun_resistance(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance] * stun_res_multiplier;
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->stun_resistance--;
            mechlab_update(s);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance + 1] * stun_res_multiplier;
        pilot->money -= price;
        pilot->stun_resistance++;
        mechlab_update(s);
    }
    lab_menu_focus_stun_resistance(c, true, userdata);
}

void lab_menu_customize_check_stun_resistance_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance] * stun_res_multiplier;
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance + 1] * stun_res_multiplier;
        if(price < 0 || price > pilot->money || pilot->stun_resistance + 1 > max_stun_res[pilot->har_id]) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_trade(component *c, void *userdata) {
    scene *s = userdata;
    trnmenu_set_submenu(c->parent, lab_menu_trade_create(s));
}

void lab_menu_customize_check_trade_robot(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);

    int trade_value = calculate_trade_value(p1->pilot);
    bool trades = false;
    for(int i = 0; i < 11; i++) {
        if(i == p1->pilot->har_id) {
            // don't trade for the current HAR
            continue;
        }
        if((p1->pilot->har_trades >> i) & 1 && har_prices[i] < trade_value + p1->pilot->money) {
            trades = true;
        }
    }
    if(!trades) {
        component_disable(c, 1);
    } else {
        component_disable(c, 0);
    }
}

static const button_details details_list[] = {
    {lab_menu_customize_color_main,      NULL,          TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED}, // Blue
    {lab_menu_customize_color_third,     NULL,          TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0,
     COM_ENABLED                                                                                                          }, // Yellow
    {lab_menu_customize_color_secondary, NULL,          TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0,
     COM_ENABLED                                                                                                          }, // Red
    {lab_menu_customize_arm_power,       "ARM POWER",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_leg_power,       "LEG POWER",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_arm_speed,       "ARM SPEED",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_leg_speed,       "LEG SPEED",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_armor,           "ARMOR",       TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_stun_resistance, "STUN RES.",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0,
     COM_ENABLED                                                                                                          },
    {lab_menu_customize_trade,           "TRADE ROBOT", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_customize_done,            "DONE",        TEXT_VERTICAL,   TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
};

static const spritebutton_tick_cb tickers[] = {NULL,
                                               NULL,
                                               NULL,
                                               lab_menu_customize_check_arm_power_price,
                                               lab_menu_customize_check_leg_power_price,
                                               lab_menu_customize_check_arm_speed_price,
                                               lab_menu_customize_check_leg_speed_price,
                                               lab_menu_customize_check_armor_price,
                                               lab_menu_customize_check_stun_resistance_price,
                                               lab_menu_customize_check_trade_robot,
                                               NULL};

void lab_menu_focus_blue(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(547));
        } else {
            mechlab_set_hint(s, lang_get(548));
        }
        label_set_text(header_label, "");
        label_set_text(details_label, "");
    }
}

void lab_menu_focus_yellow(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(551));
        } else {
            mechlab_set_hint(s, lang_get(552));
        }
        label_set_text(header_label, "");
        label_set_text(details_label, "");
    }
}

void lab_menu_focus_red(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(549));
        } else {
            mechlab_set_hint(s, lang_get(550));
        }
        label_set_text(header_label, "");
        label_set_text(details_label, "");
    }
}

static void lab_menu_focus_arm_power(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "ARM POWER:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power] * arm_leg_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->arm_power, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(553), "arm");
            mechlab_set_hint(s, tmp);
        } else {
            label_set_text(header_label, "ARM POWER:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power + 1] * arm_leg_multiplier;
            bool max_level = pilot->arm_power >= max_arm_power[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->arm_power + 1, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(554), "arm");
            mechlab_set_hint(s, tmp);
        }
    }
}

static void lab_menu_focus_leg_power(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "LEG POWER:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power] * arm_leg_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->leg_power, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(555), "leg");
            mechlab_set_hint(s, tmp);
        } else {
            label_set_text(header_label, "LEG POWER:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power + 1] * arm_leg_multiplier;
            bool max_level = pilot->leg_power >= max_leg_power[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->leg_power + 1, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(556), "leg");
            mechlab_set_hint(s, tmp);
        }
    }
}

static void lab_menu_focus_arm_speed(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "ARM SPEED:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed] * arm_leg_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->arm_speed, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(557), "arm");
            mechlab_set_hint(s, tmp);
        } else {
            label_set_text(header_label, "ARM SPEED:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed + 1] * arm_leg_multiplier;
            bool max_level = pilot->arm_speed >= max_arm_speed[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->arm_speed + 1, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(558), "arm");
            mechlab_set_hint(s, tmp);
        }
    }
}

static void lab_menu_focus_leg_speed(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "LEG SPEED:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed] * arm_leg_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->leg_speed, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(559), "leg");
            mechlab_set_hint(s, tmp);
        } else {
            label_set_text(header_label, "LEG SPEED:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed + 1] * arm_leg_multiplier;
            bool max_level = pilot->leg_speed >= max_leg_speed[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->leg_speed + 1, price_str);
                label_set_text(details_label, tmp);
            }
            snprintf(tmp, sizeof(tmp), lang_get(560), "leg");
            mechlab_set_hint(s, tmp);
        }
    }
}

static void lab_menu_focus_armor(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "ARMOR PLATE:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor] * armor_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->armor, price_str);
                label_set_text(details_label, tmp);
            }
            mechlab_set_hint(s, lang_get(561));
        } else {
            label_set_text(header_label, "ARMOR PLATE:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor + 1] * armor_multiplier;
            bool max_level = pilot->armor >= max_armor[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->armor + 1, price_str);
                label_set_text(details_label, tmp);
            }
            mechlab_set_hint(s, lang_get(562));
        }
    }
}

static void lab_menu_focus_stun_resistance(component *c, bool focused, void *userdata) {
    char tmp[200];
    char price_str[32];
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(header_label, "STUN RES.:\n\nSALES PRICE:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance] * stun_res_multiplier;
            if(price < 1) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format((int)(price * 0.85), price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->stun_resistance, price_str);
                label_set_text(details_label, tmp);
            }
            mechlab_set_hint(s, lang_get(563));
        } else {
            label_set_text(header_label, "STUN RES.:\n\nUPGRADE COST:");
            int32_t price = har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance + 1] * stun_res_multiplier;
            bool max_level = pilot->stun_resistance >= max_stun_res[pilot->har_id];
            if(price < 1 || max_level) {
                label_set_text(details_label, "Unavailable\n\nUnavailable");
            } else {
                score_format(price, price_str, sizeof(price_str));
                snprintf(tmp, sizeof(tmp), "Level %d\n\n$ %sK", pilot->stun_resistance + 1, price_str);
                label_set_text(details_label, tmp);
            }
            mechlab_set_hint(s, lang_get(564));
        }
    }
}

void lab_menu_focus_trade(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        mechlab_set_hint(s, lang_get(565));
        int trade_value = calculate_trade_value(p1->pilot);
        uint8_t trades[5];
        memset(trades, 0, sizeof(trades));
        uint8_t tradecount = 0;
        for(int i = 0; i < 11; i++) {
            if(i == p1->pilot->har_id) {
                // don't trade for the current HAR
                continue;
            }
            if((p1->pilot->har_trades >> i) & 1 && har_prices[i] < trade_value + p1->pilot->money) {
                trades[tradecount] = i;
                tradecount++;
            }
        }
        log_debug("got %d trades from the bitmask %d", tradecount, p1->pilot->har_trades);
        // check if there's anything for trade
        if(tradecount == 0) {
            label_set_text(header_label, lang_get(488));
            label_set_text(details_label, "");
        } else {
            label_set_text(header_label, lang_get(461));
            char tmp[200] = "";
            // pick 5 of however many we got
            // naturally, I unrolled this loop for performance
            if(tradecount == 1) {
                snprintf(tmp, 200, "%s", lang_get(31 + trades[0]));
            } else if(tradecount == 2) {
                snprintf(tmp, 200, "%s %s", lang_get(31 + trades[0]), lang_get(31 + trades[1]));
            } else if(tradecount == 3) {
                snprintf(tmp, 200, "%s %s %s", lang_get(31 + trades[0]), lang_get(31 + trades[1]),
                         lang_get(31 + trades[2]));
            } else if(tradecount == 4) {
                snprintf(tmp, 200, "%s %s %s %s", lang_get(31 + trades[0]), lang_get(31 + trades[1]),
                         lang_get(31 + trades[2]), lang_get(31 + trades[3]));
            } else if(tradecount == 5) {
                snprintf(tmp, 200, "%s %s %s %s %s", lang_get(31 + trades[0]), lang_get(31 + trades[1]),
                         lang_get(31 + trades[2]), lang_get(31 + trades[3]), lang_get(31 + trades[4]));
            }
            label_set_text(details_label, tmp);
        }
    }
}

void lab_menu_focus_done(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(567));
        } else {
            mechlab_set_hint(s, lang_get(568));
        }
        label_set_text(header_label, "");
        label_set_text(details_label, "");
    }
}

static const spritebutton_focus_cb focus_cbs[] = {
    lab_menu_focus_blue,      lab_menu_focus_yellow,    lab_menu_focus_red,
    lab_menu_focus_arm_power, lab_menu_focus_leg_power, lab_menu_focus_arm_speed,
    lab_menu_focus_leg_speed, lab_menu_focus_armor,     lab_menu_focus_stun_resistance,
    lab_menu_focus_trade,     lab_menu_focus_done,
};

component *lab_menu_customize_create(scene *s) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 3)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;
    animation *har_picture = &bk_get_info(s->bk_data, 5)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 0);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, false);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = TEXT_TRN_BLUE;
    tconf.cselected = TEXT_TRN_BLUE;
    tconf.cinactive = TEXT_TRN_BLUE;
    tconf.cdisabled = TEXT_TRN_BLUE;

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
        spritebutton_set_tick_cb(button, tickers[i]);

        spritebutton_set_focus_cb(button, focus_cbs[i]);

        component_tick(button);
        trnmenu_attach(menu, button);
    }

    game_player *p1 = game_state_get_player(s->gs, 0);
    sprite *bsprite = animation_get_sprite(har_picture, p1->pilot->har_id);
    component *button = spritebutton_create(&tconf, "", bsprite->data, COM_ENABLED, NULL, NULL);
    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    button->supports_select = false;
    spritebutton_set_always_display(button);
    trnmenu_attach(menu, button);

    tconf.direction = TEXT_HORIZONTAL;
    tconf.halign = TEXT_LEFT;
    tconf.valign = TEXT_TOP;
    tconf.lspacing = 2;
    tconf.cforeground = 0xA5;
    header_label = label_create(&tconf, "");
    component_set_size_hints(header_label, 90, 80);
    component_set_pos_hints(header_label, 210, 150);
    trnmenu_attach(menu, header_label);

    tconf.cforeground = 0xA7;
    details_label = label_create(&tconf, "");
    component_set_size_hints(details_label, 90, 80);
    component_set_pos_hints(details_label, 210, 158);
    trnmenu_attach(menu, details_label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}

int sell_highest_value_upgrade(sd_pilot *pilot, char *sold) {
    int32_t prices[] = {
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_power] * arm_leg_multiplier,
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->arm_speed] * arm_leg_multiplier,
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_power] * arm_leg_multiplier,
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->leg_speed] * arm_leg_multiplier,
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->stun_resistance] * stun_res_multiplier,
        har_upgrade_price[pilot->har_id] * upgrade_level_mutiplier[pilot->armor] * armor_multiplier,
    };
    int max_idx = -1;
    int32_t max_price = 0;
    for(unsigned i = 0; i < N_ELEMENTS(prices); ++i) {
        if(prices[i] > max_price) {
            max_price = prices[i];
            max_idx = i;
        }
    }
    switch(max_idx) {
        case 0:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d ARM POWER", pilot->arm_power + 1);
            pilot->arm_power--;
            break;
        case 1:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d ARM SPEED", pilot->arm_speed + 1);
            pilot->arm_speed--;
            break;
        case 2:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d LEG POWER", pilot->leg_power + 1);
            pilot->leg_power--;
            break;
        case 3:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d LEG SPEED", pilot->leg_speed + 1);
            pilot->leg_speed--;
            break;
        case 4:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d STUN RES.", pilot->stun_resistance + 1);
            pilot->stun_resistance--;
            break;
        case 5:
            pilot->money += (int32_t)(prices[max_idx] * 0.85);
            snprintf(sold, SOLD_BUF_SIZE, "LEVEL %d ARMOR PLATE", pilot->armor + 1);
            pilot->armor--;
            break;
        default:
            return 0;
    }
    return 1;
}
