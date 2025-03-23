#include "game/scenes/mechlab/lab_dash_newplayer.h"
#include "game/gui/label.h"
#include "game/gui/spriteimage.h"
#include "game/gui/textinput.h"
#include "game/gui/xysizer.h"
#include "resources/bk.h"
#include "resources/languages.h"

component *lab_dash_newplayer_create(scene *s, newplayer_widgets *nw) {
    component *xy = xysizer_create();

    // Background name box
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    sprite *msprite = animation_get_sprite(main_sheets, 5);
    xysizer_attach(xy, spriteimage_create(msprite->data), msprite->pos.x, msprite->pos.y, -1, -1);

    // Dialog text
    component *label = label_create(lang_get(192));
    label_set_font(label, FONT_SMALL);
    xysizer_attach(xy, label, 110, 43, 100, 50);

    // Input field
    nw->input = textinput_create(16, "Name", "");
    textinput_set_font(nw->input, FONT_SMALL);
    textinput_set_horizontal_align(nw->input, TEXT_ALIGN_LEFT);
    component_select(nw->input, true);
    textinput_enable_background(nw->input, false);
    xysizer_attach(xy, nw->input, 114, 62, 120, 8);

    return xy;
}
