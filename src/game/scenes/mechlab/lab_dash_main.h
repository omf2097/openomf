#ifndef LAB_DASH_MAIN_H
#define LAB_DASH_MAIN_H

#include "formats/pilot.h"
#include "game/gui/component.h"
#include "game/protos/scene.h"

// For easy access to components
typedef struct {
    scene *scene;
    sd_pilot *pilot;
    list *savegames;
    int16_t index;
    component *photo;
    component *power;
    component *agility;
    component *endurance;
    component *arm_power;
    component *arm_speed;
    component *leg_power;
    component *leg_speed;
    component *armor;
    component *stun_resistance;
    component *name;
    component *money;
    component *rank;
    component *wins;
    component *losses;
    component *tournament;
    component *har_name;
    component *har_moves;
} dashboard_widgets;

void lab_dash_main_photo_select(component *c, void *userdata);
void lab_dash_main_photo_left(component *c, void *userdata);
void lab_dash_main_photo_right(component *c, void *userdata);

void lab_dash_main_chr_load(component *c, void *userdata);
void lab_dash_main_chr_delete(component *c, void *userdata);
void lab_dash_main_chr_left(component *c, void *userdata);
void lab_dash_main_chr_right(component *c, void *userdata);

component *lab_dash_main_create(scene *s, dashboard_widgets *dw);
void lab_dash_main_update(scene *s, dashboard_widgets *dw);

#endif // LAB_DASH_MAIN_H
