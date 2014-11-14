#ifndef _LAB_DASHBOARD_H
#define _LAB_DASHBOARD_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

// For easy access to components
typedef struct {
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
} dashboard_widgets;

component* lab_dashboard_create(scene *s, dashboard_widgets *dw);
void lab_dashboard_update(scene *s, dashboard_widgets *dw);

#endif // _LAB_DASHBOARD_H
