#include <stdlib.h>

#include "game/game_state.h"
#include "game/scenes/credits.h"
#include "utils/allocator.h"
#include "video/video.h"

typedef struct credits_local_t {
    int ticks;
} credits_local;

void credits_input_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);

    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                   i->event_data.action == ACT_PUNCH) {

                    game_state_set_next(scene->gs, SCENE_NONE);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void credits_tick(scene *scene, int paused) {
    credits_local *local = scene_get_userdata(scene);
    local->ticks++;
    if(local->ticks > 4500) {
        game_state_set_next(scene->gs, SCENE_NONE);
    }
}

void credits_free(scene *scene) {
    credits_local *local = scene_get_userdata(scene);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void credits_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    switch(id) {
        case 20:
            *m_load = 1;
            return;
    }
}

int credits_create(scene *scene) {
    credits_local *local = omf_calloc(1, sizeof(credits_local));
    local->ticks = 0;

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_dynamic_tick_cb(scene, credits_tick);
    scene_set_free_cb(scene, credits_free);
    scene_set_startup_cb(scene, credits_startup);
    scene_set_input_poll_cb(scene, credits_input_tick);

    return 0;
}
