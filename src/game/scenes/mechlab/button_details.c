#include "game/scenes/mechlab/button_details.h"
#include "utils/log.h"

component *sprite_button_from_details(const button_details *details, const char *text, const surface *img,
                                      void *userdata) {
    text = text != NULL ? text : details->text;

    log_debug("sprite_button_from_details(%s)", text);

    component *b = spritebutton_create(text, img, details->disabled, details->cb, userdata);
    spritebutton_set_vertical_align(b, details->valign);
    spritebutton_set_horizontal_align(b, details->halign);
    spritebutton_set_text_direction(b, details->dir);
    spritebutton_set_text_margin(b, details->margin);
    return b;
}
