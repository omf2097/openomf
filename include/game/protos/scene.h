#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include "resources/ids.h"
#include "resources/bk_loader.h"

typedef struct scene_t {
    bk bk_data;

    void *userdata;
    void (*free)(struct scene_t *scene);
    int (*event)(struct scene_t *scene, SDL_Event *event);
    void (*render)(struct scene_t *scene);
    void (*tick)(struct scene_t *scene);
    void (*act)(struct scene_t *scene, controller *ctrl, int action);
} scene;

int scene_create(scene *scene, unsigned int scene_id);
void scene_free(scene *scene);
int scene_event(scene *scene, SDL_Event *event);
void scene_render(scene *scene);
void scene_tick(scene *scene);
void scene_act(scene *scene, controller *ctrl, int action);
int scene_is_valid(int id);

#endif // _SCENE_H
