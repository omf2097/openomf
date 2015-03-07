#include "game/scenes/mechlab/lab_newplayer.h"
#include "game/gui/xysizer.h"
#include "game/gui/spriteimage.h"
#include "resources/bk.h"

component* lab_newplayer_create(scene *s) {
    component *xy = xysizer_create();

    // Background name box
    animation *main_sheets = &bk_get_info(&s->bk_data, 1)->ani;
    sprite *msprite = animation_get_sprite(main_sheets, 5);
    xysizer_attach(xy, spriteimage_create(msprite->data), msprite->pos.x, msprite->pos.y, -1, -1);

    return xy;
}