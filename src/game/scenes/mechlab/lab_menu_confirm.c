#include "game/scenes/mechlab/lab_menu_confirm.h"
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

void lab_menu_confirm_yes(component *c, void *userdata) {
    lab_menu_select_t *sel = userdata;
    sel->cb(c, sel->data);
    trnmenu_finish(c->parent);
}

void lab_menu_confirm_no(component *c, void *userdata) {
    lab_menu_select_t *sel = userdata;
    sel->cb(c, sel->data);
    trnmenu_finish(c->parent);
}

component *lab_menu_confirm_create(scene *s, lab_menu_select_cb yes, void *yesdata, lab_menu_select_cb no, void *nodata,
                                   char *title) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 6)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 3);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, false);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = TEXT_TRN_BLUE;
    tconf.cselected = TEXT_TRN_BLUE;
    tconf.cinactive = TEXT_TRN_BLUE;
    tconf.cdisabled = TEXT_TRN_BLUE;

    tconf.valign = TEXT_MIDDLE;
    tconf.halign = TEXT_CENTER;
    tconf.padding.top = 0;
    tconf.padding.bottom = 0;
    tconf.padding.left = 0;
    tconf.padding.right = 0;
    tconf.direction = TEXT_HORIZONTAL;

    lab_menu_select_t *yesgo = omf_calloc(1, sizeof(lab_menu_select_t));
    yesgo->cb = yes;
    yesgo->data = yesdata;

    sprite *bsprite = animation_get_sprite(main_buttons, 0);
    component *button =
        spritebutton_create(&tconf, lang_get(229), bsprite->data, COM_ENABLED, lab_menu_confirm_yes, yesgo);

    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button);

    lab_menu_select_t *nogo = omf_calloc(1, sizeof(lab_menu_select_t));
    nogo->cb = no;
    nogo->data = nodata;

    bsprite = animation_get_sprite(main_buttons, 1);
    button = spritebutton_create(&tconf, lang_get(228), bsprite->data, COM_ENABLED, lab_menu_confirm_no, nogo);

    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button);

    // Add text label
    tconf.cforeground = TEXT_MEDIUM_GREEN;
    component *label = label_create(&tconf, title);
    component_set_pos_hints(label, 10, 155);
    component_set_size_hints(label, 300, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
