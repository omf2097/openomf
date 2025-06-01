#ifndef BUTTON_DETAILS_H
#define BUTTON_DETAILS_H

#include "game/gui/spritebutton.h"
#include "game/gui/text/enums.h"

typedef struct button_details {
    spritebutton_click_cb cb;
    const char *text;
    text_row_direction dir;
    text_horizontal_align halign;
    text_vertical_align valign;
    text_margin margin;
    bool disabled;
} button_details;

component *sprite_button_from_details(const button_details *details, const char *text, surface *img,
                                      void *userdata);

#endif // BUTTON_DETAILS_H
