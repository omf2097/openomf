#include <SDL2/SDL.h>
#include <stdlib.h>

#include "game/scenes/intro.h"
#include "game/game_state.h"
#include "video/video.h"
#include "resources/ids.h"
#include "utils/log.h"

typedef struct intro_local_t {
    int ticks;
} intro_local;

int intro_event(scene *scene, SDL_Event *event) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1=NULL, *i;
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if (
                        i->event_data.action == ACT_ESC ||
                        i->event_data.action == ACT_KICK ||
                        i->event_data.action == ACT_PUNCH) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 1;
}

int intro_startup(scene *scene, int id) {
    switch(id) {
        case 25:
            return 1;
    }
    return 0;
}

void intro_tick(scene *scene, int paused) {
    intro_local *local = scene_get_userdata(scene);
    local->ticks++;
    if(local->ticks > 2500) {
        game_state_set_next(scene->gs, SCENE_MENU);
    }
}

void intro_free(scene *scene) {
    free(scene_get_userdata(scene));
}

int intro_anim_override(scene *scene, int anim_id) {
    switch(anim_id) {
        case 25:
            return RENDER_LAYER_TOP;
    }
    return -1;
}

int intro_create(scene *scene) {
    intro_local *local = malloc(sizeof(intro_local));
    local->ticks = 0;

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_dynamic_tick_cb(scene, intro_tick);
    scene_set_event_cb(scene, intro_event);
    scene_set_free_cb(scene, intro_free);
    scene_set_startup_cb(scene, intro_startup);
    scene_set_anim_prio_override_cb(scene, intro_anim_override);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_QUIRKS);

    return 0;
}
