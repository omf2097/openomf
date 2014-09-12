#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/menu/textbutton.h"
#include "game/menu/menu_background.h"
#include "video/video.h"
#include "audio/sound.h"
#include "utils/log.h"
#include "utils/compat.h"

typedef struct textbutton_t {
    char *text;
    font *font;
    int ticks;
    int dir;
    int border_enabled;
    int border_created;
    color border_color;
    surface border;
} textbutton;

void textbutton_create(component *c, font *font, const char *text) {
    component_create(c);

    textbutton *tb = malloc(sizeof(textbutton));
    tb->text = strdup(text);
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    tb->border_enabled = 0;
    tb->border_created = 0;
    component_set_obj(c, tb);

    component_set_render_cb(c, textbutton_render);
    component_set_action_cb(c, textbutton_action);
    component_set_tick_cb(c, textbutton_tick);
}

void textbutton_free(component *c) {
    textbutton *tb = c->obj;
    if(tb->border_created) {
        surface_free(&tb->border);
    }
    free(tb->text);
    free(tb);
    component_free(c);
}

void textbutton_render(component *c) {
    textbutton *tb = c->obj;
    int chars = strlen(tb->text);
    int width = chars*tb->font->w;
    int xoff = (c->w - width)/2;
    if(c->selected) {
        int t = tb->ticks / 2;
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(80 - t, 220 - t*2, 80 - t, 255));
    } else if (c->disabled) {
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    } else {
        font_render(tb->font, tb->text, c->x + xoff, c->y, color_create(0, 121, 0, 255));
    }
    if(tb->border_enabled) {
        video_render_sprite(&tb->border, c->x + xoff-4, c->y-2, BLEND_ALPHA, 0);
    }
}

int textbutton_action(component *c, int action) {
    // Handle selection
    if(action == ACT_KICK || action == ACT_PUNCH) {
        component_click(c);
        sound_play(20, 0.5f, 0.0f, 2.0f);
        return 0;
    }
    return 1;
}

void textbutton_tick(component *c) {
    textbutton *tb = c->obj;
    if(!tb->dir) {
        tb->ticks++;
    } else {
        tb->ticks--;
    }
    if(tb->ticks > 120) {
        tb->dir = 1;
    }
    if(tb->ticks == 0) {
        tb->dir = 0;
    }
}

void textbutton_set_border(component *c, color col) {
    textbutton *tb = c->obj;
    tb->border_enabled = 1;
    tb->border_color = col;
    if(tb->border_created) {
        // destroy the old border first
        surface_free(&tb->border);
    }

    // create new border
    int chars = strlen(tb->text);
    int width = chars*tb->font->w;
    menu_background_border_create(&tb->border, width+6, tb->font->h+3);
    tb->border_created = 1;
}

void textbutton_remove_border(component *c) {
    textbutton *tb = c->obj;
    tb->border_enabled = 0;
}
