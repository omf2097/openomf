#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/lab_menu_pilotselect.h"
#include "game/common_defines.h"
#include "game/gui/label.h"
#include "game/gui/pilotpic.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "resources/ids.h"
#include "utils/log.h"

void lab_menu_pilotselect_choose(component *c, void *userdata) {
    DEBUG("CHOOSE PILOT");
    // TODO we need to store the photo id in the pilot
    // but none of the callbacks have a reference to both the
    // scene, which contains the player, and the dashboard widgers
    // which contain the photo id
    //game_player *player1 = game_state_get_player(s->gs, 0);
    //player1->pilot.photo_id = pilotpic_selected(dw->photo);
    trnmenu_finish(c->parent);
}

void lab_menu_pilotselect_left(component *c, void *userdata) {
    DEBUG("PILOT -1");
    dashboard_widgets *dw = userdata;
    pilotpic_prev(dw->photo);
}

void lab_menu_pilotselect_right(component *c, void *userdata) {
    DEBUG("PILOT +1");
    dashboard_widgets *dw = userdata;
    pilotpic_next(dw->photo);
}

int lab_menu_pilotselected(dashboard_widgets *dw) {
    return pilotpic_selected(dw->photo);
}

static const button_details details_list[] = {
    {lab_menu_pilotselect_choose, NULL, TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_pilotselect_left,   NULL,     TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_pilotselect_right,  NULL,     TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
};

component *lab_menu_pilotselect_create(scene *s, dashboard_widgets *dw) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 7)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 4);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    game_player *p1 = game_state_get_player(s->gs, 0);
    pilotpic_select(dw->photo, PIC_PLAYERS, p1->pilot->photo_id);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = color_create(0, 0, 123, 255);

    // Init GUI buttons with locations from the "select" button sprites
    // TODO the left-right buttons apply on focus, not select
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
            spritebutton_create(&tconf, i == 0 ? lang_get(223) : NULL, bsprite->data, COM_ENABLED, details_list[i].cb, i == 0 ? (void*)s : (void*)dw);
        component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
        trnmenu_attach(menu, button);
    }

    // Add text label
    tconf.cforeground = color_create(0, 121, 0, 255);
    // TODO interpolate %s in the string here with blank
    char tmp[200];
    snprintf(tmp, 200, lang_get(224), p1->pilot->name);
    component *label = label_create(&tconf, tmp);
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
