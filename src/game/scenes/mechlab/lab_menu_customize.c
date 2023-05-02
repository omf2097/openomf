#include "game/scenes/mechlab/lab_menu_customize.h"
#include "formats/pilot.h"
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

// TODO put these somewhere central
static uint8_t max_arm_speed[11] = {6, 8, 4, 6, 9, 7, 8, 6, 9, 6, 7};
static uint8_t max_leg_speed[11] = {8, 9, 5, 6, 8, 8, 7, 6, 7, 5, 6};
static uint8_t max_arm_power[11] = {5, 5, 9, 8, 4, 6, 6, 5, 5, 6, 7};
static uint8_t max_leg_power[11] = {6, 6, 8, 4, 5, 7, 5, 7, 6, 7, 7};

// I don't care anymore, sorry
static component *label1;
static component *label2;

// negative values means the upgrade is unavailable at that level
int32_t arm_leg_prices[11][10] = {
  // jaguar
    {0, 760,  2280, 5320, 9120,  13680, 22800, 38000, 57000, -1    },
 // shadow
    {0, 800,  2400, 5600, 9600,  14400, 24000, 40000, 60000, 96000 },
 // thorn
    {0, 700,  2100, 4900, 8400,  12600, 21000, 35000, 52500, 84000 },
 // pyros
    {0, 800,  2400, 5600, 9600,  14400, 24000, 40000, 60000, -1    },
 // electra
    {0, 1000, 3000, 7000, 12000, 18000, 30000, 50000, 75000, 120000},
 // katana
    {0, 660,  1980, 4620, 7920,  11880, 19800, 3300,  49500, -1    },
 // shredder
    {0, 840,  2520, 5880, 10080, 15120, 25200, 42000, 63000, -1    },
 // flail
    {0, 740,  2220, 5180, 8880,  13320, 22200, 37000, -1,    -1    },
 // gargoyle
    {0, 900,  2700, 6300, 10800, 16200, 27000, 45000, 67500, 108000},
 // chronos
    {0, 720,  2160, 5040, 8640,  12960, 21600, 36000, -1,    -1    },
 // nova
    {0, 1400, 4200, 9800, 16800, 25200, 42000, 70000, -1,    -1    }
};

uint32_t har_price[11] = {20000, 36000, 26000, 28000, 29000, 32000, 25000, 30000, 24000, 22000, 75000};

int32_t stun_resistance_prices[11][10] = {
  // jaguar
    {0, 1140, 3420, 7980,  13680, 20520, 34200, 57000, 85500, -1},
 // shadow
    {0, 1200, 3600, 8400,  14400, 21600, 36000, -1,    -1,    -1},
 // thorn
    {0, 1050, 3150, 7350,  12600, 18900, 31500, 52500, 78750, -1},
 // pyros
    {0, 1200, 3600, 8400,  14400, 21600, 36000, -1,    -1,    -1},
 // electra
    {0, 1500, 4500, 10500, 18000, 27000, 45000, 75000, -1,    -1},
 // katana
    {0, 990,  2970, 6930,  11880, 17820, 29700, -1,    -1,    -1},
 // shredder
    {0, 1260, 3780, 8820,  15120, 22680, 37800, -1,    -1,    -1},
 //  flail
    {0, 1110, 3330, 7770,  13320, 19980, 33300, 55500, -1,    -1},
 // gargoyle
    {0, 1350, 4050, 9450,  16200, 24300, 40500, 67500, -1,    -1},
 // chronos
    {0, 1080, 3240, 7560,  12960, 19440, 32400, 54000, -1,    -1},
 // nova
    {0, 2100, 6300, 14700, 25200, 37800, 63000, -1,    -1,    -1}
};

int32_t armor_prices[][10] = {
  // jaguar
    {0, 1900, 5700,  13300, 22800, 34200, -1,     -1,     -1,     -1    },
 // shadow
    {0, 2000, 6000,  14000, 24000, 36000, 60000,  100000, -1,     -1    },
 // thorn
    {0, 1750, 5250,  12250, 21000, 31500, 52500,  87500,  -1,     -1    },
 // pyros
    {0, 2000, 6000,  14000, 24000, 36000, 60000,  100000, 150000, -1    },
 // electra
    {0, 2500, 7500,  17500, 30000, 45000, 75000,  -1,     -1,     -1    },
 // katana
    {0, 1650, 4950,  11550, 19800, 29700, 49500,  82500,  123750, -1    },
 // shredder
    {0, 2100, 6300,  14700, 25200, 37800, 63000,  -1,     -1,     -1    },
 // flail
    {0, 1850, 5550,  12950, 22200, 33300, 55500,  92500,  138750, 222000},
 // gargoyle
    {0, 2250, 6750,  15750, 27000, 40500, 67500,  -1,     -1,     -1    },
 // chronos
    {0, 1800, 5400,  12600, 21600, 32400, 54000,  -1,     -1,     -1    },
 // nova
    {0, 3500, 10500, 24500, 42000, 63000, 105000, 175000, -1,     -1    }
};

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
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->arm_power--;
            mechlab_update(s);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power + 1];
        pilot->money -= price;
        pilot->arm_power++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_arm_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power + 1];
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
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->leg_power--;
            mechlab_update(s);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power + 1];
        pilot->money -= price;
        pilot->leg_power++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_leg_power_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power + 1];
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
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->arm_speed--;
            mechlab_update(s);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed + 1];
        pilot->money -= price;
        pilot->arm_speed++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_arm_speed_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed + 1];
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
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->leg_speed--;
            mechlab_update(s);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed + 1];
        pilot->money -= price;
        pilot->leg_speed++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_leg_speed_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed + 1];
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
        int32_t price = armor_prices[pilot->har_id][pilot->armor];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->armor--;
            mechlab_update(s);
        }
    } else {
        int32_t price = armor_prices[pilot->har_id][pilot->armor + 1];
        pilot->money -= price;
        pilot->armor++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_armor_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = armor_prices[pilot->har_id][pilot->armor];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = armor_prices[pilot->har_id][pilot->armor + 1];
        if(price < 0 || price > pilot->money) {
            component_disable(c, 1);
        }
    }
}

void lab_menu_customize_stun_resistance(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance];
        if(price > 0) {
            pilot->money += price * 0.85;
            pilot->stun_resistance--;
            mechlab_update(s);
        }
    } else {
        int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance + 1];
        pilot->money -= price;
        pilot->stun_resistance++;
        mechlab_update(s);
    }
}

void lab_menu_customize_check_stun_resistance_price(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    sd_pilot *pilot = game_player_get_pilot(p1);
    if(mechlab_get_selling(s)) {
        int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance];
        if(price < 1) {
            component_disable(c, 1);
        }
    } else {
        int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance + 1];
        if(price < 0 || price > pilot->money) {
            component_disable(c, 1);
        }
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
    {NULL,                               "TRADE ROBOT", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
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
                                               NULL,
                                               NULL};

void lab_menu_focus_blue(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(547));
        } else {
            mechlab_set_hint(s, lang_get(548));
        }
        label_set_text(label1, "");
        label_set_text(label2, "");
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
        label_set_text(label1, "");
        label_set_text(label2, "");
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
        label_set_text(label1, "");
        label_set_text(label2, "");
    }
}

void lab_menu_focus_arm_power(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "ARM POWER:\n\nSALES PRICE:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->arm_power, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(553)); // TODO sprintf arm/leg
        } else {
            label_set_text(label1, "ARM POWER:\n\nUPGRADE COST:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_power + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->arm_power + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(554)); // TODO sprintf arm/leg
        }
    }
}

void lab_menu_focus_leg_power(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "LEG POWER:\n\nSALES PRICE:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->leg_power, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(555)); // TODO sprintf arm/leg
        } else {
            label_set_text(label1, "LEG POWER:\n\nUPGRADE COST:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_power + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->leg_power + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(556)); // TODO sprintf arm/leg
        }
    }
}

void lab_menu_focus_arm_speed(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "ARM SPEED:\n\nSALES PRICE:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->arm_speed, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(557)); // TODO sprintf arm/leg
        } else {
            label_set_text(label1, "ARM SPEED:\n\nUPGRADE COST:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->arm_speed + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->arm_speed + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(558)); // TODO sprintf arm/leg
        }
    }
}

void lab_menu_focus_leg_speed(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "LEG SPEED:\n\nSALES PRICE:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->leg_speed, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(559)); // TODO sprintf arm/leg
        } else {
            label_set_text(label1, "LEG SPEED:\n\nUPGRADE COST:");
            int32_t price = arm_leg_prices[pilot->har_id][pilot->leg_speed + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->leg_speed + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(560)); // TODO sprintf arm/leg
        }
    }
}

void lab_menu_focus_armor(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "ARMOR PLATE:\n\nSALES PRICE:");
            int32_t price = armor_prices[pilot->har_id][pilot->armor];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->armor, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(561));
        } else {
            label_set_text(label1, "ARMOR PLATE:\n\nUPGRADE COST:");
            int32_t price = armor_prices[pilot->har_id][pilot->armor + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->armor + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(562));
        }
    }
}

void lab_menu_focus_stun_resistance(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        if(mechlab_get_selling(s)) {
            label_set_text(label1, "STUN RES.:\n\nSALES PRICE:");
            int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->stun_resistance, (int)(price * 0.85));
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(563));
        } else {
            label_set_text(label1, "STUN RES.:\n\nUPGRADE COST:");
            int32_t price = stun_resistance_prices[pilot->har_id][pilot->stun_resistance + 1];
            if(price < 1) {
                label_set_text(label2, "Unavailable\n\nUnavailable");
            } else {
                char tmp[200];
                snprintf(tmp, 200, "Level %d\n\n$ %dK", pilot->stun_resistance + 1, price);
                label_set_text(label2, tmp);
            }
            mechlab_set_hint(s, lang_get(564));
        }
    }
}

void lab_menu_focus_trade(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(565));
        } else {
            mechlab_set_hint(s, lang_get(566));
        }
    }

    // TODO check if there's anything for trade
    label_set_text(label1, lang_get(488));
    label_set_text(label2, "");
}

void lab_menu_focus_done(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        if(mechlab_get_selling(s)) {
            mechlab_set_hint(s, lang_get(567));
        } else {
            mechlab_set_hint(s, lang_get(568));
        }
        label_set_text(label1, "");
        label_set_text(label2, "");
    }
}

static const spritebutton_focus_cb focus_cbs[] = {
    lab_menu_focus_blue,      lab_menu_focus_yellow,    lab_menu_focus_red,
    lab_menu_focus_arm_power, lab_menu_focus_leg_power, lab_menu_focus_arm_speed,
    lab_menu_focus_leg_speed, lab_menu_focus_armor,     lab_menu_focus_stun_resistance,
    lab_menu_focus_trade,     lab_menu_focus_done,
};

component *lab_menu_customize_create(scene *s) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 3)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 0);
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
        spritebutton_set_tick_cb(button, tickers[i]);

        spritebutton_set_focus_cb(button, focus_cbs[i]);

        component_tick(button);
        trnmenu_attach(menu, button);
    }

    tconf.direction = TEXT_HORIZONTAL;
    tconf.halign = TEXT_LEFT;
    tconf.valign = TEXT_TOP;
    tconf.lspacing = 2;
    tconf.cforeground = color_create(0, 200, 0, 255);
    label1 = label_create(&tconf, "");
    component_set_size_hints(label1, 90, 80);
    component_set_pos_hints(label1, 210, 150);
    trnmenu_attach(menu, label1);

    tconf.cforeground = color_create(0, 255, 0, 255);
    label2 = label_create(&tconf, "");
    component_set_size_hints(label2, 90, 80);
    component_set_pos_hints(label2, 210, 158);
    trnmenu_attach(menu, label2);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
