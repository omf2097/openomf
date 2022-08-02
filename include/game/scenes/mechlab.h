#ifndef MECHLAB_H
#define MECHLAB_H

#include "game/protos/scene.h"
#include "game/scenes/mechlab/lab_dash_main.h"

typedef enum
{
    DASHBOARD_NONE,
    DASHBOARD_STATS,
    DASHBOARD_NEW,
    DASHBOARD_SELECT_NEW_PIC,
    DASHBOARD_SELECT_TOURNAMENT,
} dashboard_type;

int mechlab_create(scene *scene);
void mechlab_select_dashboard(scene *scene, dashboard_type type);
dashboard_widgets* mechlab_get_dashboard_widgets(scene *scene);

#endif // MECHLAB_H
