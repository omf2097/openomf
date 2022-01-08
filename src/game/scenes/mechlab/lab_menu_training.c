#include "game/scenes/mechlab/lab_menu_training.h"
#include "game/common_defines.h"
#include "game/gui/sizer.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text_render.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "utils/log.h"

void lab_menu_training_done(component *c, void *userdata) { trnmenu_finish(c->parent); }

static const button_details details_list[] = {
    {NULL, "POWER", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {NULL, "AGILITY", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {NULL, "ENDUR.", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_TOP, 2, 0, 0, 0, COM_ENABLED},
    {lab_menu_training_done, "DONE", TEXT_VERTICAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0,
     COM_ENABLED},
};

component *lab_menu_training_create(scene *s) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 9)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 1);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.cforeground = color_create(0, 0, 123, 255);

    // Init GUI buttons with locations from the "select" button sprites
    for (int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        tconf.valign = details_list[i].valign;
        tconf.halign = details_list[i].halign;
        tconf.padding.top = details_list[i].top;
        tconf.padding.bottom = details_list[i].bottom;
        tconf.padding.left = details_list[i].left;
        tconf.padding.right = details_list[i].right;
        tconf.direction = details_list[i].dir;

        sprite *bsprite = animation_get_sprite(main_buttons, i);
        component *button = spritebutton_create(&tconf, details_list[i].text, bsprite->data,
                                                COM_ENABLED, details_list[i].cb, s);
        component_set_size_hints(button, bsprite->data->w, bsprite->data->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
        trnmenu_attach(menu, button);
    }

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
