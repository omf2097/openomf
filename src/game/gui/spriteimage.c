#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/gui/spriteimage.h"
#include "game/gui/widget.h"
#include "game/gui/sizer.h"
#include "video/video.h"
#include "audio/sound.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/compat.h"
#include "utils/miscmath.h"

typedef struct {
    surface *img;
} spriteimage;

static void spriteimage_render(component *c) {
    spriteimage *sb = widget_get_obj(c);
    video_render_sprite(sb->img, c->x, c->y, BLEND_ALPHA, 0);
}

static void spriteimage_free(component *c) {
    spriteimage *sb = widget_get_obj(c);
    omf_free(sb);
}

component* spriteimage_create(surface *img) {
    component *c = widget_create();
    component_disable(c, 1);
    c->supports_focus = 0;
    c->supports_disable = 1;
    c->supports_select = 0;

    spriteimage *sb = omf_calloc(1, sizeof(spriteimage));
    sb->img = img;
    widget_set_obj(c, sb);

    widget_set_render_cb(c, spriteimage_render);
    widget_set_free_cb(c, spriteimage_free);

    return c;
}
