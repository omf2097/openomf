#include <SDL.h>
#include <stdlib.h>

#include "game/game_state.h"
#include "game/scenes/intro.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct intro_local_t {
    int ticks;
} intro_local;

void intro_input_tick(scene *scene) {
    ctrl_event *p1 = NULL, *i;
    game_state_menu_poll(scene->gs, &p1);

    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                   i->event_data.action == ACT_PUNCH) {

                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void intro_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    switch(id) {
        case 25:
            *m_load = 1;
            return;
    }
}

void intro_tick(scene *scene, int paused) {
    intro_local *local = scene_get_userdata(scene);
    local->ticks++;
    if(local->ticks > 2500) {
        game_state_set_next(scene->gs, SCENE_MENU);
    }
}

void intro_free(scene *scene) {
    intro_local *local = scene_get_userdata(scene);
    omf_free(local);
    scene_set_userdata(scene, local);
}

int intro_anim_override(scene *scene, int anim_id) {
    switch(anim_id) {
        case 25:
            return RENDER_LAYER_TOP;
    }
    return -1;
}

int intro_create(scene *scene) {
    intro_local *local = omf_calloc(1, sizeof(intro_local));
    local->ticks = 0;

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_dynamic_tick_cb(scene, intro_tick);
    scene_set_input_poll_cb(scene, intro_input_tick);
    scene_set_free_cb(scene, intro_free);
    scene_set_startup_cb(scene, intro_startup);
    scene_set_anim_prio_override_cb(scene, intro_anim_override);

    return 0;
}
