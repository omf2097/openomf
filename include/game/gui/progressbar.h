#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "video/color.h"
#include "game/gui/component.h"

typedef struct {
    color border_topleft_color;
    color border_bottomright_color;
    color bg_color;
    color bg_color_alt;
    color int_topleft_color;
    color int_bottomright_color;
    color int_bg_color;
} progressbar_theme;

extern const progressbar_theme _progressbar_theme_health;
extern const progressbar_theme _progressbar_theme_endurance;
extern const progressbar_theme _progressbar_theme_melee;

#define PROGRESSBAR_THEME_HEALTH _progressbar_theme_health
#define PROGRESSBAR_THEME_ENDURANCE _progressbar_theme_endurance
#define PROGRESSBAR_THEME_MELEE _progressbar_theme_melee

#define PROGRESSBAR_LEFT 0
#define PROGRESSBAR_RIGHT 1

component* progressbar_create(progressbar_theme theme, int orientation, int percentage);
void progressbar_set_progress(component *bar, int percentage);
void progressbar_set_flashing(component *bar, int flashing, int rate);

#endif // PROGRESSBAR_H
