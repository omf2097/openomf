#ifndef LAB_MENU_TRNSELECT_H
#define LAB_MENU_TRNSELECT_H

#include "formats/tournament.h"
#include "game/gui/component.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab/lab_dash_trnselect.h"

component *lab_menu_trnselect_create(scene *s, trnselect_widgets *tw);
sd_tournament_file* lab_menu_trnselected(trnselect_widgets *tw);

#endif // LAB_MENU_trnselect_H
