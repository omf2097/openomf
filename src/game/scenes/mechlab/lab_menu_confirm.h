#ifndef LAB_MENU_CONFIRM_H
#define LAB_MENU_CONFIRM_H

#include "formats/tournament.h"
#include "game/gui/component.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab/lab_menu_select.h"

component *lab_menu_confirm_create(scene *s, lab_menu_select_cb yes, void *yesdata, lab_menu_select_cb no, void *nodata,
                                   char *title);

#endif // LAB_MENU_CONFIRM_H
