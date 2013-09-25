#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "game/scene.h"
#include "game/scenes/credits.h"

int credits_init(scene *scene) {
    return 0;
}

void credits_free(scene *scene) {}
void credits_render(scene *scene) {}
void credits_tick(scene *scene) {}

int credits_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            scene->next_id = SCENE_NONE;
            return 1;
        }
        break;
    }
    return 1;
}

void credits_load(scene *scene) {
    scene_set_init_cb(scene, credits_init);
    scene_set_render_cb(scene, credits_render);
    scene_set_event_cb(scene, credits_event);
    scene_set_free_cb(scene, credits_free);
    scene_set_tick_cb(scene, credits_tick);
}

