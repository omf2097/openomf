#include "game/scenes/mechlab/lab_menu_select.h"
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
#include "resources/trnmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

void lab_menu_select_choose(component *c, void *userdata) {
    lab_menu_select_t *sel = userdata;
    sel->cb(c, sel->data);
    trnmenu_finish(c->parent);
}

void lab_menu_select_left(component *c, void *userdata) {
    lab_menu_select_t *left = userdata;
    left->cb(c, left->data);
}

void lab_menu_select_right(component *c, void *userdata) {
    lab_menu_select_t *right = userdata;
    right->cb(c, right->data);
}

component *lab_menu_select_create(scene *s, lab_menu_select_cb select, void *selectdata, lab_menu_select_cb left,
                                  void *leftdata, lab_menu_select_cb right, void *rightdata, int title) {
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

    lab_menu_select_t *selector = omf_calloc(1, sizeof(lab_menu_select_t));
    selector->cb = select;
    selector->data = selectdata;

    tconf.valign = TEXT_MIDDLE;
    tconf.halign = TEXT_CENTER;
    tconf.padding.top = 0;
    tconf.padding.bottom = 0;
    tconf.padding.left = 0;
    tconf.padding.right = 0;
    tconf.direction = TEXT_HORIZONTAL;

    sprite *bsprite = animation_get_sprite(main_buttons, 0);
    component *button =
        spritebutton_create(&tconf, lang_get(223), bsprite->data, COM_ENABLED, lab_menu_select_choose, selector);

    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button);

    lab_menu_select_t *goleft = omf_calloc(1, sizeof(lab_menu_select_t));
    goleft->cb = left;
    goleft->data = leftdata;

    bsprite = animation_get_sprite(main_buttons, 1);
    button = spritebutton_create(&tconf, NULL, bsprite->data, COM_ENABLED, lab_menu_select_left, goleft);
    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button);

    lab_menu_select_t *goright = omf_calloc(1, sizeof(lab_menu_select_t));
    goright->cb = right;
    goright->data = rightdata;

    bsprite = animation_get_sprite(main_buttons, 2);
    button = spritebutton_create(&tconf, NULL, bsprite->data, COM_ENABLED, lab_menu_select_right, goright);
    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button);

    // TODO the left-right buttons apply on focus, not select

    // Add text label
    tconf.cforeground = COLOR_DARK_GREEN;
    // TODO interpolate %s in the string here with blank
    component *label = label_create(&tconf, lang_get(title));
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
