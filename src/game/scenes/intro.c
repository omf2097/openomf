#include "game/scenes/intro.h"
#include "resources/ids.h"
#include "game/game_state.h"
#include "utils/log.h"
#include <SDL2/SDL.h>

int intro_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            game_state_set_next(SCENE_MENU);
            return 1;
        }
        break;
    }
    return 1;
}

int intro_create(scene *scene) {
    scene_set_event_cb(scene, intro_event);
    return 0;
}
