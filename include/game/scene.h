#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "video/video.h"

enum {
    SCENE_INTRO = 0,
    SCENE_MENU,
    SCENE_END,
    SCENE_MECHLAB,
    SCENE_NEWSROOM,
    SCENE_TOURNAMENT,
    SCENE_ARENA0,
    SCENE_ARENA1,
    SCENE_ARENA2,
    SCENE_ARENA3,
    SCENE_ARENA4,
    SCENE_ARENA5,
    SCENE_NONE
};

typedef struct scene_t scene;

typedef struct scene_t {
    sd_bk_file *bk;
    unsigned int this_id;
    unsigned int next_id;
    unsigned int ticks;
    texture background;
    
    int (*event)(scene *scene, SDL_Event *event);
    void (*render)(scene *scene);
} scene;

int scene_load(scene *scene, unsigned int scene_id);
void scene_free(scene *scene);
int scene_handle_event(scene *scene, SDL_Event *event);
void scene_render(scene *scene);

#endif // _SCENE_H