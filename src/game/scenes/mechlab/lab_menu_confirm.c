#include "game/scenes/mechlab/lab_menu_confirm.h"
#include "game/gui/label.h"
#include "game/gui/spritebutton.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "resources/bk.h"
#include "resources/languages.h"
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

    lab_menu_select_t *yesgo = omf_calloc(1, sizeof(lab_menu_select_t));
    yesgo->cb = yes;
    yesgo->data = yesdata;

    sprite *bsprite = animation_get_sprite(main_buttons, 0);
    component *button_yes = spritebutton_create(lang_get(229), bsprite->data, false, lab_menu_confirm_yes, yesgo);
    spritebutton_set_font(button_yes, FONT_SMALL);
    component_set_pos_hints(button_yes, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button_yes);

    lab_menu_select_t *nogo = omf_calloc(1, sizeof(lab_menu_select_t));
    nogo->cb = no;
    nogo->data = nodata;

    bsprite = animation_get_sprite(main_buttons, 1);
    component *button_no = spritebutton_create(lang_get(228), bsprite->data, false, lab_menu_confirm_no, nogo);
    spritebutton_set_font(button_no, FONT_SMALL);
    component_set_pos_hints(button_no, bsprite->pos.x, bsprite->pos.y);
    trnmenu_attach(menu, button_no);

    // Add text label
    component *label = label_create(title);
    component_set_pos_hints(label, 10, 155);
    component_set_size_hints(label, 300, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
