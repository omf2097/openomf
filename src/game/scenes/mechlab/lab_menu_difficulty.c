#include "game/scenes/mechlab/lab_menu_difficulty.h"
#include "game/scenes/mechlab/lab_menu_tournament.h"
#include "game/scenes/mechlab.h"
#include "game/gui/label.h"
#include "game/gui/pilotpic.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "utils/log.h"

void lab_menu_difficulty_choose(component *c, void *userdata) {
    INFO("Difficulty not yet implemented, bypassing!");
    scene *s = userdata;
    dashboard_widgets *dw = mechlab_get_dashboard_widgets(userdata);
    trnmenu_set_submenu(c->parent, lab_menu_tournament_create(s, dw));
    mechlab_select_dashboard(s, DASHBOARD_SELECT_TOURNAMENT);
}

static const button_details details_list[] = {
    {lab_menu_difficulty_choose, "ALUMINUM\n(EASY)", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_difficulty_choose, "IRON\n(MEDIUM)",   TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_difficulty_choose, "STEEL\n(HARD)",    TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
    {lab_menu_difficulty_choose, "HEAVY\nMETAL",     TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0, COM_ENABLED},
};

component *lab_menu_difficulty_create(scene *s, dashboard_widgets *dw) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 2)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 6);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = color_create(0, 0, 123, 255);

    // Init GUI buttons with locations from the "difficulty" button sprites
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
        trnmenu_attach(menu, button);
    }

    // Add text label
    tconf.cforeground = color_create(0, 121, 0, 255);
    component *label = label_create(&tconf, "SELECT A DIFFICULTY LEVEL");
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
