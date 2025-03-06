#include <stdio.h>

#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "game/scenes/mechlab/lab_menu_confirm.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/scenes/mechlab/lab_menu_trade.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/allocator.h"
#include "utils/log.h"

void lab_menu_trade_done(component *menu, component *submenu) {
    scene *s = trnmenu_get_userdata(submenu);
    log_debug("trade done");
    game_player *p1 = game_state_get_player(s->gs, 0);
    if(p1->pilot != &p1->chr->pilot) {
        omf_free(p1->pilot);
        p1->pilot = &p1->chr->pilot;
        mechlab_update(s);
    }
}

void confirm_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    int trade_value = calculate_trade_value(&p1->chr->pilot);
    int har_value = har_price(p1->pilot->har_id);
    p1->chr->pilot.money += trade_value - har_value;
    p1->chr->pilot.har_id = p1->pilot->har_id;
    p1->chr->pilot.leg_speed = 0;
    p1->chr->pilot.arm_speed = 0;
    p1->chr->pilot.leg_power = 0;
    p1->chr->pilot.arm_power = 0;
    p1->chr->pilot.armor = 0;
    p1->chr->pilot.stun_resistance = 0;
    omf_free(p1->pilot);
    p1->pilot = &p1->chr->pilot;
    mechlab_update(s);
    trnmenu_finish(c->parent);
}

void cancel_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    omf_free(p1->pilot);
    p1->pilot = &p1->chr->pilot;
    mechlab_update(s);
    trnmenu_finish(c->parent);
}

void lab_menu_trade(component *c, void *userdata) {
    scene *s = userdata;
    game_player *p1 = game_state_get_player(s->gs, 0);
    char tmp[100];
    int trade_value = calculate_trade_value(&p1->chr->pilot);
    int har_value = har_price(p1->pilot->har_id);
    if(trade_value == har_value) {
        snprintf(tmp, 100, lang_get(520), lang_get(31 + p1->chr->pilot.har_id), lang_get(31 + p1->pilot->har_id));
    } else if(trade_value > har_value) {
        char price[15];
        snprintf(price, 15, "$ %dK", trade_value - har_value);
        snprintf(tmp, 100, lang_get(518), lang_get(31 + p1->chr->pilot.har_id), lang_get(31 + p1->pilot->har_id),
                 price);
    } else if(trade_value + p1->pilot->money > har_value) {
        char price[15];
        snprintf(price, 15, "$ %dK", har_value - trade_value);
        snprintf(tmp, 100, lang_get(519), lang_get(31 + p1->chr->pilot.har_id), price,
                 lang_get(31 + p1->pilot->har_id));
    }

    component *menu = lab_menu_confirm_create(s, confirm_trade, s, cancel_trade, s, tmp);
    trnmenu_set_userdata(menu, s);
    trnmenu_set_submenu_done_cb(menu, lab_menu_trade_done);
    trnmenu_finish(c->parent);
    trnmenu_set_submenu(c->parent->parent, menu);
}

void lab_menu_trade_for_jaguar_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 0;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_shadow_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 1;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_thorn_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 2;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_pyros_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 3;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_electra_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 4;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_katana_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 5;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_shredder_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 6;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_flail_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 7;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_gargoyle_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 8;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_chronos_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 9;
        mechlab_update(s);
    }
}

void lab_menu_trade_for_nova_focus(component *c, bool focused, void *userdata) {
    if(focused) {
        scene *s = userdata;
        game_player *p1 = game_state_get_player(s->gs, 0);
        sd_pilot *pilot = game_player_get_pilot(p1);
        pilot->har_id = 10;
        mechlab_update(s);
    }
}

static const button_details details_list[] = {
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_trade, "", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
};

static const spritebutton_focus_cb focus_cbs[] = {
    lab_menu_trade_for_jaguar_focus,   lab_menu_trade_for_shadow_focus,  lab_menu_trade_for_thorn_focus,
    lab_menu_trade_for_pyros_focus,    lab_menu_trade_for_electra_focus, lab_menu_trade_for_katana_focus,
    lab_menu_trade_for_shredder_focus, lab_menu_trade_for_flail_focus,   lab_menu_trade_for_gargoyle_focus,
    lab_menu_trade_for_chronos_focus,  lab_menu_trade_for_nova_focus};

component *lab_menu_trade_create(scene *s) {
    // animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 13)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    game_player *p1 = game_state_get_player(s->gs, 0);

    p1->pilot = omf_calloc(1, sizeof(sd_pilot));
    memcpy(p1->pilot, &p1->chr->pilot, sizeof(sd_pilot));
    p1->pilot->leg_speed = 0;
    p1->pilot->arm_speed = 0;
    p1->pilot->leg_power = 0;
    p1->pilot->arm_power = 0;
    p1->pilot->armor = 0;
    p1->pilot->stun_resistance = 0;

    int x = 24;
    int y = 148;
    // Initialize menu, and set button sheet
    component *menu = trnmenu_create(NULL, x, y, false);
    // sprite *msprite = animation_get_sprite(main_sheets, 1);
    // component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

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
        if(i == p1->pilot->har_id || 0 == ((p1->pilot->har_trades >> i) & 1)) {
            log_debug("skipping har %d", i, p1->pilot->har_trades);
            continue;
        }
        log_debug("adding button");
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
        component_set_pos_hints(button, x + bsprite->pos.x, y + bsprite->pos.y);

        x += bsprite->data->w;

        spritebutton_set_focus_cb(button, focus_cbs[i]);
        spritebutton_set_always_display(button);

        trnmenu_attach(menu, button);
    }

    trnmenu_set_userdata(menu, s);
    trnmenu_set_submenu_done_cb(menu, lab_menu_trade_done);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
