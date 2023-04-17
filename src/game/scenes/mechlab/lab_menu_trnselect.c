#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/lab_menu_trnselect.h"
#include "game/common_defines.h"
#include "game/gui/label.h"
#include "game/gui/trnselect.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "resources/trnmanager.h"
#include "utils/log.h"

void lab_menu_trnselect_choose(component *c, void *userdata) {
    DEBUG("CHOOSE TRN");
}

void lab_menu_trnselect_left(component *c, void *userdata) {
    DEBUG("TRN -1");
    trnselect_widgets *tw = userdata;
    trnselect_prev(tw->trnselect);
}

void lab_menu_trnselect_right(component *c, void *userdata) {
    DEBUG("TRN +1");
    trnselect_widgets *tw = userdata;
    trnselect_next(tw->trnselect);
}

static const button_details details_list[] = {
    {lab_menu_trnselect_choose, NULL, TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_trnselect_left,   NULL,     TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_trnselect_right,  NULL,     TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
};

component *lab_menu_trnselect_create(scene *s, trnselect_widgets *dw) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 7)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 4);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

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
    component *label = label_create(&tconf, lang_get(486));
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
