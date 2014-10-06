#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/gui/spritebutton.h"
#include "game/gui/widget.h"
#include "video/video.h"
#include "audio/sound.h"
#include "utils/log.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    const font *font;
    surface *img;

    int pad_top;
    int pad_bottom;
    int pad_right;
    int pad_left;
    int halign;
    int valign;

    spritebutton_click_cb click_cb;
    void *userdata;
} spritebutton;

static void spritebutton_render(component *c) {
    spritebutton *sb = widget_get_obj(c);

    int w_size = c->w - sb->pad_left - sb->pad_right;

    int width, height;
    font_get_wrapped_size(sb->font, sb->text, w_size, &width, &height);

    int xoff = 0;
    int yoff = 0;
    if(sb->halign == HALIGN_CENTER) {
        xoff = (c->w - width) / 2;
    }
    if(sb->valign == VALIGN_MIDDLE) {
        yoff = (c->h - height) / 2;
    }

    // HACK TO FIX STUPID FONT RENDERING
    // font_render_wrapped always wants to center everything, so fix offset. Bleargh.
    // TODO: Fix this!
    if(sb->halign == HALIGN_LEFT)
        xoff = -xoff;
    if(sb->halign == HALIGN_CENTER)
        xoff = 0;

    xoff += sb->pad_left;
    yoff += sb->pad_top;

    font_render_wrapped(sb->font, sb->text, c->x + xoff, c->y + yoff, w_size, color_create(0, 0, 123, 255));
    //video_render_sprite(sb->img, c->x, c->y, BLEND_ALPHA, 0);
}

static void spritebutton_free(component *c) {
    spritebutton *sb = widget_get_obj(c);
    free(sb->text);
    free(sb);
}

static int spritebutton_action(component *c, int action) {
    spritebutton *sb = widget_get_obj(c);

    // Handle selection
    if(action == ACT_KICK || action == ACT_PUNCH) {
        if(sb->click_cb) {
            sb->click_cb(c, sb->userdata);
        }
        return 0;
    }
    return 1;
}

void spritebutton_set_text_padding(component *c, int top, int bottom, int left, int right) {
    spritebutton *sb = widget_get_obj(c);
    sb->pad_top = top;
    sb->pad_bottom = bottom;
    sb->pad_left = left;
    sb->pad_right = right;
}

void spritebutton_set_text_align(component *c, int halign, int valign) {
    spritebutton *sb = widget_get_obj(c);
    sb->valign = valign;
    sb->halign = halign;
}

component* spritebutton_create(const font *font, const char *text, surface *img, int disabled, spritebutton_click_cb cb, void *userdata) {
    component *c = widget_create();
    component_disable(c, disabled);

    spritebutton *sb = malloc(sizeof(spritebutton));
    memset(sb, 0, sizeof(spritebutton));
    sb->text = strdup(text);
    sb->font = font;
    sb->click_cb = cb;
    sb->img = img;
    sb->userdata = userdata;
    widget_set_obj(c, sb);

    widget_set_render_cb(c, spritebutton_render);
    widget_set_action_cb(c, spritebutton_action);
    //widget_set_tick_cb(c, spritebutton_tick);
    widget_set_free_cb(c, spritebutton_free);

    return c;
}



