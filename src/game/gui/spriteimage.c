#include <stdlib.h>

#include "game/gui/component.h"
#include "game/gui/spriteimage.h"
#include "utils/allocator.h"
#include "video/video.h"

typedef struct spriteimage {
    const surface *img;
    bool owns_sprite;
} spriteimage;

static void spriteimage_render(component *c) {
    const spriteimage *sb = component_get_obj(c);
    const vec2i pos = component_get_pos(c);
    video_draw(sb->img, pos.x, pos.y);
}

static void spriteimage_free(component *c) {
    spriteimage *sb = component_get_obj(c);
    if(sb->owns_sprite) {
        // bypass const here
        surface_free((surface *)sb->img);
        free((surface *)sb->img);
    }
    omf_free(sb);
}

void spriteimage_set_owns_sprite(component *c, bool owns_sprite) {
    spriteimage *sb = component_get_obj(c);
    sb->owns_sprite = owns_sprite;
}

component *spriteimage_create(const surface *img) {
    component *c = component_create();
    component_disable(c, 1);
    component_set_supported(c, true, false, false);

    spriteimage *sb = omf_calloc(1, sizeof(spriteimage));
    sb->img = img;
    sb->owns_sprite = false;
    component_set_obj(c, sb);

    component_set_render_cb(c, spriteimage_render);
    component_set_free_cb(c, spriteimage_free);

    return c;
}
