#include "game/scenes/mechlab/lab_menu_difficultyselect.h"
#include "game/common_defines.h"
#include "game/gui/label.h"
#include "game/gui/portrait.h"
#include "game/gui/spritebutton.h"
#include "game/gui/text/enums.h"
#include "game/gui/trn_menu.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/button_details.h"
#include "resources/bk.h"
#include "resources/languages.h"
#include "utils/log.h"

void lab_menu_difficultyselect_aluminium(component *c, void *userdata) {
    log_debug("ALUMINIUM");
    scene *s = userdata;
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->difficulty = 0;
    trnmenu_finish(c->parent);
}

void lab_menu_difficultyselect_iron(component *c, void *userdata) {
    log_debug("IRON");
    scene *s = userdata;
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->difficulty = 1;
    trnmenu_finish(c->parent);
}

void lab_menu_difficultyselect_steel(component *c, void *userdata) {
    log_debug("STEEL");
    scene *s = userdata;
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->difficulty = 2;
    trnmenu_finish(c->parent);
}

void lab_menu_difficultyselect_heavy(component *c, void *userdata) {
    log_debug("HEAVY");
    scene *s = userdata;
    game_player *player1 = game_state_get_player(s->gs, 0);
    player1->pilot->difficulty = 3;
    trnmenu_finish(c->parent);
}

static const button_details details_list[] = {
    {lab_menu_difficultyselect_aluminium,
     NULL, TEXT_ROW_HORIZONTAL,
     ALIGN_TEXT_CENTER, ALIGN_TEXT_MIDDLE,
     {0, 0, 0, 0},
     false},
    {lab_menu_difficultyselect_iron,
     NULL, TEXT_ROW_HORIZONTAL,
     ALIGN_TEXT_CENTER, ALIGN_TEXT_MIDDLE,
     {0, 0, 0, 0},
     false},
    {lab_menu_difficultyselect_steel,
     NULL, TEXT_ROW_HORIZONTAL,
     ALIGN_TEXT_CENTER, ALIGN_TEXT_MIDDLE,
     {0, 0, 0, 0},
     false},
    {lab_menu_difficultyselect_heavy,
     NULL, TEXT_ROW_HORIZONTAL,
     ALIGN_TEXT_CENTER, ALIGN_TEXT_MIDDLE,
     {0, 0, 0, 0},
     false},
};

component *lab_menu_difficultyselect_create(scene *s) {
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    animation *main_buttons = &bk_get_info(s->bk_data, 2)->ani;
    animation *hand_of_doom = &bk_get_info(s->bk_data, 29)->ani;

    // Initialize menu, and set button sheet
    sprite *msprite = animation_get_sprite(main_sheets, 6);
    component *menu = trnmenu_create(msprite->data, msprite->pos.x, msprite->pos.y, false);

    // Init GUI buttons with locations from the "select" button sprites
    for(int i = 0; i < animation_get_sprite_count(main_buttons); i++) {
        sprite *button_sprite = animation_get_sprite(main_buttons, i);
        component *button = sprite_button_from_details(&details_list[i], lang_get(444 + i), button_sprite->data, s);
        spritebutton_set_font(button, FONT_SMALL);
        spritebutton_set_text_color(button, TEXT_TRN_BLUE);
        component_set_pos_hints(button, button_sprite->pos.x, button_sprite->pos.y);
        trnmenu_attach(menu, button);
    }

    // Add text label
    component *label = label_create("SELECT A DIFFICULTY LEVEL");
    label_set_text_color(label, TEXT_MEDIUM_GREEN);
    label_set_font(label, FONT_SMALL);
    component_set_pos_hints(label, 87, 155);
    component_set_size_hints(label, 150, 10);
    trnmenu_attach(menu, label);

    // Bind hand animation
    trnmenu_bind_hand(menu, hand_of_doom, s->gs);

    return menu;
}
