#include <SDL2/SDL.h>
#include <stdlib.h>

#include "game/scenes/intro.h"
#include "resources/ids.h"
#include "game/game_state.h"
#include "utils/log.h"

typedef struct intro_local_t {
    int ticks;
} intro_local;

int intro_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            game_state_set_next(scene->gs, SCENE_MENU);
            return 1;
        }
        break;
    }
    return 1;
}

int intro_startup(scene *scene, int id) {
    switch(id) {
        case 25:
            return 1;
    }
    return 0;
}

void intro_tick(scene *scene) {
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
    scene_set_userdata(scene, local);
    scene_set_tick_cb(scene, intro_tick);
    scene_set_event_cb(scene, intro_event);
    scene_set_free_cb(scene, intro_free);
    scene_set_startup_cb(scene, intro_startup);
    scene_set_anim_prio_override_cb(scene, intro_anim_override);
    return 0;
}
