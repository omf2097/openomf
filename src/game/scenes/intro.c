#include "game/scene.h"
#include "game/scenes/intro.h"
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>

int intro_event(scene *scene, SDL_Event *event) {
    return 1;
}

void intro_render(scene *scene) {

}

void intro_load(scene *scene) {
    scene->event = intro_event;
    scene->render = intro_render;
}
