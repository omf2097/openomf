#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "audio/music.h"
#include "game/scene.h"
#include "game/scenes/menu.h"

int menu_event(scene *scene, SDL_Event *event) {
    return 1;
}

void menu_render(scene *scene) {

}

void menu_load(scene *scene) {
    scene->event = menu_event;
    scene->render = menu_render;
    if(!music_playing()) {
        music_play("resources/MENU.PSM");
    }
}
