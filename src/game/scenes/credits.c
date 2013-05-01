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

void credits_deinit(scene *scene) {
}

void credits_tick(scene *scene) {

}

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

void credits_render(scene *scene) {

}

void credits_load(scene *scene) {
    scene->event = credits_event;
    scene->render = credits_render;
    scene->init = credits_init;
    scene->deinit = credits_deinit;
    scene->tick = credits_tick;
}

