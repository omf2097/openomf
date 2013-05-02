#include "game/scene.h"
#include "game/scenes/arena.h"
#include "audio/music.h"
#include "game/har.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <shadowdive/shadowdive.h>

int arena_init(scene *scene) {
    music_stop();
    switch (scene->bk->file_id) {
        case 8:
            music_play("resources/ARENA0.PSM");
            break;
        case 16:
            music_play("resources/ARENA1.PSM");
            break;
        case 32:
            music_play("resources/ARENA2.PSM");
            break;
        case 64:
            music_play("resources/ARENA3.PSM");
            break;
        case 128:
            music_play("resources/ARENA4.PSM");
            break;
    }
    
    // Load some har on the arena
    har *h1 = malloc(sizeof(har));
    har_load(h1, scene->bk->palettes[0], scene->bk->soundtable, "resources/FIGHTR0.AF");
    scene_set_player1_har(scene, h1);
    return 0;
}

void arena_deinit(scene *scene) {
    scene_set_player1_har(scene, NULL);
    music_stop();
}

void arena_tick(scene *scene) {

}

int arena_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            scene->next_id = SCENE_MENU;
            return 1;
        }
        break;
    }
    return 1;
}

void arena_render(scene *scene) {

}

void arena_load(scene *scene) {
    scene->event = arena_event;
    scene->render = arena_render;
    scene->init = arena_init;
    scene->deinit = arena_deinit;
    scene->tick = arena_tick;
}
