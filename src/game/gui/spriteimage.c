#include <stdlib.h>

#include "game/gui/sizer.h"
#include "game/gui/spriteimage.h"
#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "video/video.h"

typedef struct spriteimage {
    const surface *img;
} spriteimage;

static void spriteimage_render(component *c) {
    spriteimage *sb = widget_get_obj(c);
    video_draw(sb->img, c->x, c->y);
}

static void spriteimage_free(component *c) {
    spriteimage *sb = widget_get_obj(c);
    omf_free(sb);
}

component *spriteimage_create(const surface *img) {
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
