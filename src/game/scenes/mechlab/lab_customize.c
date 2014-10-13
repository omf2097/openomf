#include "game/scenes/mechlab/lab_customize.h"
#include "game/gui/trn_menu.h"
#include "game/gui/spritebutton.h"
#include "game/gui/sizer.h"
#include "game/gui/text_render.h"
#include "resources/bk.h"
#include "game/common_defines.h"
#include "utils/log.h"

typedef struct {
    spritebutton_click_cb cb;
    const char *text;
    text_direction dir;
    text_halign halign;
    text_valign valign;
    int top;
    int bottom;
    int left;
    int right;
} button_details;

void lab_customize_done(component *c, void *userdata) {
    trnmenu_finish(c->parent);
}

static const button_details details_list[] = {
    {NULL, NULL, TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0}, // Blue
    {NULL, NULL, TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0}, // Yellow
    {NULL, NULL, TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0}, // Red
    {NULL, "ARM POWER", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "LEG POWER", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "ARM SPEED", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "LEG SPEED", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "ARMOR", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "STUN RES.", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {NULL, "TRADE ROBOT", TEXT_HORIZONTAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
    {lab_customize_done, "DONE", TEXT_VERTICAL, TEXT_CENTER, TEXT_MIDDLE, 0, 0, 0, 0},
};

component* lab_customize_create(scene *s) {
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(&s->bk_data, 3)->ani;
    animation *hand_of_doom = &bk_get_info(&s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 0);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y);

    // Default text configuration
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        sprite *bsprite = animation_get_sprite(main_buttons, i);
        surface *bsurface = bsprite->data;
        component *button = spritebutton_create(&font_small, details_list[i].text, bsurface, COM_ENABLED, details_list[i].cb, s);
        tconf.valign = details_list[i].valign;
        tconf.halign = details_list[i].halign;
        tconf.padding.top = details_list[i].top;
        tconf.padding.bottom = details_list[i].bottom;
        tconf.padding.left = details_list[i].left;
        tconf.padding.right = details_list[i].right;
        tconf.direction = details_list[i].dir;
        spritebutton_set_text_style(button, &tconf);
        component_set_size_hints(button, bsurface->w, bsurface->h);
        component_set_pos_hints(button, bsprite->pos.x, bsprite->pos.y);
        trnmenu_attach(menu, button);
    }

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
