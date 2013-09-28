#include "game/scenes/credits.h"
#include "resources/ids.h"
#include "game/game_state.h"

int credits_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            game_state_set_next(SCENE_NONE);
            return 1;
        }
        break;
    }
    return 1;
}

int credits_create(scene *scene) {
    scene_set_event_cb(scene, credits_event);
    return 0;
}

