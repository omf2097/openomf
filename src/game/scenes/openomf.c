#include <SDL.h>
#include <stdlib.h>

#include "game/game_state.h"
#include "game/scenes/openomf.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct openomf_local_t {
    int ticks;
} openomf_local;

void openomf_input_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);

    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if (i) {
        do {
            if (i->type == EVENT_TYPE_ACTION) {
                if (i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                    i->event_data.action == ACT_PUNCH) {

                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while ((i = i->next));
    }
    controller_free_chain(p1);
}

void openomf_tick(scene *scene, int paused) {
    openomf_local *local = scene_get_userdata(scene);
    local->ticks++;
    if (local->ticks > 140) {
        game_state_set_next(scene->gs, SCENE_INTRO);
    }
}

void openomf_free(scene *scene) {
    openomf_local *local = scene_get_userdata(scene);
    omf_free(local);
    scene_set_userdata(scene, local);
}

int openomf_create(scene *scene) {
    openomf_local *local = omf_calloc(1, sizeof(openomf_local));
    local->ticks = 0;

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_dynamic_tick_cb(scene, openomf_tick);
    scene_set_input_poll_cb(scene, openomf_input_tick);
    scene_set_free_cb(scene, openomf_free);

    // Don't render background on its own layer
    // Fix for some additive blending tricks.
    video_render_bg_separately(false);

    return 0;
}
