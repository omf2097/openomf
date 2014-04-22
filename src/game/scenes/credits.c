#include <stdlib.h>

#include "game/scenes/credits.h"
#include "game/game_state.h"
#include "video/video.h"
#include "resources/ids.h"

typedef struct credits_local_t {
    int ticks;
} credits_local;

int credits_event(scene *scene, SDL_Event *event) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1=NULL, *i;
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if (
                        i->event_data.action == ACT_ESC) {
                    game_state_set_next(scene->gs, SCENE_NONE);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 1;
}

void credits_tick(scene *scene, int paused) {
    credits_local *local = scene_get_userdata(scene);
    local->ticks++;
    if(local->ticks > 4500) {
        game_state_set_next(scene->gs, SCENE_NONE);
    }
}

void credits_free(scene *scene) {
    free(scene_get_userdata(scene));
}

int credits_create(scene *scene) {
    credits_local *local = malloc(sizeof(credits_local));
    local->ticks = 0;

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_tick_cb(scene, credits_tick);
    scene_set_free_cb(scene, credits_free);
    scene_set_event_cb(scene, credits_event);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_HW);

    return 0;
}
