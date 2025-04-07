#include "game/scenes/mechlab/lab_menu_select.h"
#include "game/gui/label.h"
#include "game/gui/spritebutton.h"
#include "game/gui/trn_menu.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/allocator.h"

void lab_menu_select_choose(component *c, void *userdata) {
    lab_menu_select_t *sel = userdata;
    sel->cb(c, sel->data);
    trnmenu_finish(c->parent);
}

void lab_menu_focus_left(component *c, bool focused, void *userdata) {
    if(focused) {
        component_disable(c, 1);
        lab_menu_select_t *left = userdata;
        bool en = left->cb(c, left->data);
        if(left->button) {
            component_disable(left->button, en ? 0 : 1);
        }
    } else {
        component_disable(c, 0);
    }
}

void lab_menu_focus_right(component *c, bool focused, void *userdata) {
    if(focused) {
        component_disable(c, 1);
        lab_menu_select_t *right = userdata;
        bool en = right->cb(c, right->data);
        if(right->button) {
            component_disable(right->button, en ? 0 : 1);
        }
    } else {
        component_disable(c, 0);
    }
}

component *lab_menu_select_create(scene *s, lab_menu_select_cb select, void *selectdata, lab_menu_select_cb left,
                                  void *leftdata, lab_menu_select_cb right, void *rightdata, const char *title,
                                  bool return_hand) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 7)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 4);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, return_hand);

    lab_menu_select_t *selector = omf_calloc(1, sizeof(lab_menu_select_t));
    selector->cb = select;
    selector->data = selectdata;

    sprite *bsprite = animation_get_sprite(main_buttons, 0);
    component *sel_button = spritebutton_create(lang_get(223), bsprite->data, false, lab_menu_select_choose, selector);
    spritebutton_set_font(sel_button, FONT_SMALL);
    spritebutton_set_text_color(sel_button, TEXT_TRN_BLUE);

    component_set_size_hints(sel_button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(sel_button, bsprite->pos.x, bsprite->pos.y);
    spritebutton_set_free_userdata(sel_button, true);
    trnmenu_attach(menu, sel_button);

    lab_menu_select_t *goleft = omf_calloc(1, sizeof(lab_menu_select_t));
    goleft->cb = left;
    goleft->data = leftdata;
    goleft->button = sel_button;

    bsprite = animation_get_sprite(main_buttons, 1);
    component *button = spritebutton_create(NULL, bsprite->data, false, NULL, goleft);
    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    spritebutton_set_focus_cb(button, lab_menu_focus_left);
    spritebutton_set_free_userdata(button, true);
    trnmenu_attach(menu, button);

    lab_menu_select_t *goright = omf_calloc(1, sizeof(lab_menu_select_t));
    goright->cb = right;
    goright->data = rightdata;
    goright->button = sel_button;

    bsprite = animation_get_sprite(main_buttons, 2);
    button = spritebutton_create(NULL, bsprite->data, false, NULL, goright);
    component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
    component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
    spritebutton_set_focus_cb(button, lab_menu_focus_right);
    spritebutton_set_free_userdata(button, true);
    trnmenu_attach(menu, button);

    // Add text label
    component *label = label_create_title(title);
    label_set_font(label, FONT_SMALL);
    label_set_text_color(label, TEXT_MEDIUM_GREEN);
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
