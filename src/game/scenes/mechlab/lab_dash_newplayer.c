#include "game/scenes/mechlab/lab_dash_newplayer.h"
#include "game/gui/label.h"
#include "game/gui/spriteimage.h"
#include "game/gui/textinput.h"
#include "game/gui/xysizer.h"
#include "resources/bk.h"
#include "resources/languages.h"

component *lab_dash_newplayer_create(scene *s, newplayer_widgets *nw) {
    component *xy = xysizer_create();

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = TEXT_TRN_BLUE;
    tconf.cselected = TEXT_TRN_BLUE;
    tconf.cinactive = TEXT_TRN_BLUE;
    tconf.cdisabled = TEXT_TRN_BLUE;

    // Background name box
    animation *main_sheets = &bk_get_info(s->bk_data, 1)->ani;
    sprite *msprite = animation_get_sprite(main_sheets, 5);
    xysizer_attach(xy, spriteimage_create(msprite->data), msprite->pos.x, msprite->pos.y, -1, -1);

    // Dialog text
    xysizer_attach(xy, label_create(&tconf, lang_get(192)), 110, 43, 100, 50);

    // Input field
    tconf.cforeground = TEXT_MEDIUM_GREEN;
    nw->input = textinput_create(&tconf, "Name", NULL, "");
    component_select(nw->input, 1);
    textinput_enable_background(nw->input, 0);
    textinput_set_max_chars(nw->input, 8);
    xysizer_attach(xy, nw->input, 101, 62, 120, 8);

    return xy;
}
