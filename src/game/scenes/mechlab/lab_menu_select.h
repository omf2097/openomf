#ifndef LAB_MENU_SELECT_H
#define LAB_MENU_SELECT_H

#include "formats/tournament.h"
#include "game/gui/component.h"
#include "game/protos/scene.h"

typedef void (*lab_menu_select_cb)(component *c, void *userdata);

component *lab_menu_select_create(scene *s, lab_menu_select_cb select, void *selectdata, lab_menu_select_cb left,
                                  void *leftdata, lab_menu_select_cb right, void *rightdata, int title);

typedef struct {
    lab_menu_select_cb cb;
    void *data;
} lab_menu_select_t;

#endif // LAB_MENU_SELECT_H
