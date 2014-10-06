#include "game/scenes/mechlab/lab_main.h"
#include "game/gui/trn_menu.h"
#include "game/gui/spritebutton.h"
#include "resources/bk.h"
#include "game/gui/sizer.h"
#include "utils/log.h"

typedef struct {
    const char *text;
    int halign;
    int valign;
    int top;
    int bottom;
    int left;
    int right;
} button_details;

static const button_details details_list[] = {
    {"ARENA", HALIGN_CENTER, VALIGN_TOP, 2, 0, 0, 0},
    {"TRAINING COURSES", HALIGN_CENTER, VALIGN_MIDDLE, 0, 0, 20, 0},
    {"BUY", HALIGN_CENTER, VALIGN_TOP, 2, 0, 0, 0},
    {"SELL", HALIGN_CENTER, VALIGN_TOP, 2, 0, 0, 0},
    {"LOAD", HALIGN_CENTER, VALIGN_MIDDLE, 0, 0, 14, 0},
    {"NEW", HALIGN_CENTER, VALIGN_MIDDLE, 0, 0, 14, 0},
    {"DELETE", HALIGN_CENTER, VALIGN_MIDDLE, 0, 0, 14, 0},
    {"SIM", HALIGN_CENTER, VALIGN_TOP, 2, 0, 0, 0},
    {"QUIT", HALIGN_CENTER, VALIGN_TOP, 0, 0, 0, 0},
    {"NEW TOURNAMENT", HALIGN_CENTER, VALIGN_MIDDLE, 0, 0, 0, 0},
};

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
        component *button = spritebutton_create(&font_small, details_list[i].text, bsurface, COM_ENABLED, NULL, NULL);
        spritebutton_set_text_align(button, details_list[i].halign, details_list[i].valign);
        spritebutton_set_text_padding(button, details_list[i].top, details_list[i].bottom, details_list[i].left, details_list[i].right);
        component_set_size_hints(button, bsurface->w, bsurface->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
        trnmenu_attach(menu, button);
    }

    return menu;
}
