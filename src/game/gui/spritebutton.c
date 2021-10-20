#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "game/gui/spritebutton.h"
#include "game/gui/widget.h"
#include "game/gui/sizer.h"
#include "video/video.h"
#include "video/color.h"
#include "audio/sound.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/compat.h"
#include "utils/miscmath.h"

typedef struct {
    char *text;
    text_settings tconf;
    surface *img;
    surface *disabled_img;
    int active;

    spritebutton_click_cb click_cb;
    void *userdata;
} spritebutton;

static void spritebutton_render(component *c) {
    spritebutton *sb = widget_get_obj(c);
    sizer *s = component_get_obj(c->parent);
    int opacity = clamp(s->opacity * 255, 0, 255);
    if(c->is_disabled) {
        video_render_sprite_flip_scale_opacity_tint(
            sb->img, c->x, c->y, BLEND_ALPHA, 0, 0, 1.0, opacity, color_create(128,128,128,255));
    }
    else if(sb->active > 0) {
        video_render_sprite(sb->img, c->x, c->y, BLEND_ALPHA, 0);
    }
    if(sb->text) {
        sb->tconf.opacity = opacity;
        text_render(&sb->tconf, c->x, c->y, c->w, c->h, sb->text);
    }
}

static void spritebutton_free(component *c) {
    spritebutton *sb = widget_get_obj(c);
    free(sb->text);
    free(sb);
}

static void spritebutton_tick(component *c) {
    spritebutton *sb = widget_get_obj(c);
    if(sb->active > 0) {
        sb->active--;
    }
}

static int spritebutton_action(component *c, int action) {
    spritebutton *sb = widget_get_obj(c);
    if(c->is_disabled) {
        return 1;
    }

    // Handle selection
    if(action == ACT_KICK || action == ACT_PUNCH) {
        sb->active = 10;
        if(sb->click_cb) {
            sb->click_cb(c, sb->userdata);
        }
        return 0;
    }
    return 1;
}

component* spritebutton_create(
        const text_settings *tconf,
        const char *text, surface *img,
        int disabled,
        spritebutton_click_cb cb,
        void *userdata) 
{
    component *c = widget_create();
    component_disable(c, disabled);

    spritebutton *sb = omf_calloc(1, sizeof(spritebutton));
    if(text != NULL)
        sb->text = strdup(text);
    memcpy(&sb->tconf, tconf, sizeof(text_settings));
    sb->click_cb = cb;
    sb->img = img;
    sb->userdata = userdata;
    widget_set_obj(c, sb);

    widget_set_render_cb(c, spritebutton_render);
    widget_set_action_cb(c, spritebutton_action);
    widget_set_tick_cb(c, spritebutton_tick);
    widget_set_free_cb(c, spritebutton_free);

    return c;
}
