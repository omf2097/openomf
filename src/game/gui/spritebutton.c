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

    spritebutton_click_cb click_cb;
    void *userdata;
} spritebutton;

static void spritebutton_render(component *c) {
    spritebutton *sb = widget_get_obj(c);
    (void)(sb);
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



