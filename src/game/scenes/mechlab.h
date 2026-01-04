#ifndef MECHLAB_H
#define MECHLAB_H

#include "formats/chr.h"
#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "game/protos/scene.h"

#define MECHLAB_DARK_GREEN 165
#define MECHLAB_BRIGHT_GREEN 167
#define MECHLAB_YELLOW 207

typedef enum
{
    DASHBOARD_NONE,
    DASHBOARD_STATS,
    DASHBOARD_NEW_PLAYER,
    DASHBOARD_SELECT_NEW_PIC,
    DASHBOARD_SELECT_DIFFICULTY,
    DASHBOARD_SELECT_TOURNAMENT,
    DASHBOARD_SIM,
} dashboard_type;

int mechlab_create(scene *scene);
void mechlab_update(scene *scene);

void mechlab_load_har(scene *scene, sd_pilot *pilot);

void mechlab_enter_trnselect_menu(scene *s);
component *mechlab_chrload_menu_create(scene *scene);
component *mechlab_chrdelete_menu_create(scene *scene);
component *mechlab_sim_menu_create(scene *scene);
void mechlab_open_popup(scene *s, char const *message);

void mechlab_select_dashboard(scene *scene, dashboard_type type);

void mechlab_set_selling(scene *scene, bool selling);
bool mechlab_get_selling(scene *scene);

void mechlab_set_hint(scene *scene, const char *hint);
void mechlab_spin_har(scene *scene, bool to_spin_or_not_to_spin);

sd_chr_enemy *mechlab_next_opponent(scene *scene);

gui_frame *mechlab_get_frame(scene *scene);

#endif // MECHLAB_H
