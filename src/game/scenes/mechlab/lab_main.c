#include "game/scenes/mechlab/lab_main.h"
#include "game/gui/trn_menu.h"
#include "game/gui/spritebutton.h"
#include "resources/bk.h"
#include "game/gui/sizer.h"
#include "utils/log.h"

component* lab_main_create(scene *s) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 8)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 2);
    component* menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        sprite *bsprite = animation_get_sprite(main_buttons, i);
        surface *bsurface = bsprite->data;
        component *button = spritebutton_create(&font_small, "", bsurface, COM_ENABLED, NULL, NULL);
        component_set_size_hints(button, bsurface->w, bsurface->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
        trnmenu_attach(menu, button);
    }

    return menu;
}
