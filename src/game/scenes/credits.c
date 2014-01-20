#include <stdlib.h>

#include "game/scenes/credits.h"
#include "game/game_state.h"
#include "video/video.h"
#include "resources/ids.h"

typedef struct credits_local_t {
    int ticks;
} credits_local;

int credits_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            game_state_set_next(scene->gs, SCENE_NONE);
            return 1;
        }
        break;
    }
    return 1;
}

void credits_tick(scene *scene) {
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

