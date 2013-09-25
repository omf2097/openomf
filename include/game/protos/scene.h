#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include "game/protos/object.h"
#include "resources/bk.h"

typedef struct scene_t scene;

typedef void (*scene_free_cb)(struct scene_t *scene);
typedef int (*scene_event_cb)(struct scene_t *scene, SDL_Event *event);
typedef void (*scene_render_cb)(struct scene_t *scene);
typedef void (*scene_tick_cb)(struct scene_t *scene);

struct scene_t {
    bk bk_data;
    void *game_state;
    void *userdata;
    scene_free_cb free;
    scene_event_cb event;
    scene_render_cb render;
    scene_tick_cb tick;
};

int scene_create(scene *scene, void *game_state, unsigned int scene_id);
void scene_free(scene *scene);
int scene_event(scene *scene, SDL_Event *event);
void scene_render(scene *scene);
void scene_tick(scene *scene);
int scene_is_valid(int id);

void scene_set_userdata(scene *scene, void *userdata);
void* scene_get_userdata(scene *scene);

void scene_load_new_scene(scene *scene, unsigned int scene_id);
void scene_add_object(scene *scene, object *obj);

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc);
void scene_set_event_cb(scene *scene, scene_event_cb cbfunc);
void scene_set_render_cb(scene *scene, scene_render_cb cbfunc);
void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc);

#endif // _SCENE_H
