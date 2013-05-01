#include "game/menu/textbutton.h"
#include <stdlib.h>
#include "utils/log.h"

void textbutton_create(component *c, font *font, const char *text) {
    component_create(c);
    textbutton *tb;
    tb = malloc(sizeof(textbutton));
    tb->text = text;
    tb->font = font;
    c->obj = tb;
    c->render = textbutton_render;
    c->event = textbutton_event;
}

void textbutton_free(component *c) {
    textbutton *tb = c->obj;
    free(tb);
    component_free(c);
}

void textbutton_render(component *c) {
    textbutton *tb = c->obj;
    font_render(tb->font, tb->text, c->x, c->y, 50, 205, 50);
}

void textbutton_event(component *c) {

}

