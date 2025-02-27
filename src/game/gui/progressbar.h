#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "game/gui/component.h"
#include "video/color.h"

typedef struct {
    uint8_t border_topleft_color;
    uint8_t border_bottomright_color;
    uint8_t bg_color;
    uint8_t bg_color_alt;
    uint8_t int_topleft_color;
    uint8_t int_bottomright_color;
    uint8_t int_bg_color;
} progressbar_theme;

extern const progressbar_theme _progressbar_theme_health;
extern const progressbar_theme _progressbar_theme_endurance;
extern const progressbar_theme _progressbar_theme_melee;

#define PROGRESSBAR_THEME_HEALTH _progressbar_theme_health
#define PROGRESSBAR_THEME_ENDURANCE _progressbar_theme_endurance
#define PROGRESSBAR_THEME_MELEE _progressbar_theme_melee

#define PROGRESSBAR_LEFT 0
#define PROGRESSBAR_RIGHT 1

component *progressbar_create(progressbar_theme theme, int orientation, int percentage);
void progressbar_set_progress(component *bar, int percentage, bool animate);
void progressbar_set_flashing(component *bar, int flashing, int rate);

#endif // PROGRESSBAR_H
