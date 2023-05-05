#ifndef LAB_MENU_CUSTOMIZE_H
#define LAB_MENU_CUSTOMIZE_H

#include "game/gui/component.h"
#include "game/protos/scene.h"

component *lab_menu_customize_create(scene *s);
int calculate_trade_value(sd_pilot *pilot);
int har_price(int har_id);

#endif // LAB_MENU_CUSTOMIZE_H
