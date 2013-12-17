#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include "controller/controller.h"
#include "game/protos/object.h"
#include "game/game_player.h"
#include "resources/bk.h"

typedef struct scene_t scene;
typedef struct game_player_t game_player;

typedef void (*scene_free_cb)(scene *scene);
typedef int (*scene_event_cb)(scene *scene, SDL_Event *event);
typedef void (*scene_render_cb)(scene *scene);
typedef void (*scene_render_overlay_cb)(scene *scene);
typedef void (*scene_tick_cb)(scene *scene);
typedef void (*scene_input_tick_cb)(scene *scene);
typedef int (*scene_startup_cb)(scene *scene, int anim_id);

struct scene_t {
    int id;
    bk bk_data;
    void *userdata;
    object background;
    scene_free_cb free;
    scene_event_cb event;
    scene_render_cb render;
    scene_render_overlay_cb render_overlay;
    scene_tick_cb tick;
    scene_input_tick_cb input_tick;
    scene_startup_cb startup;
    image shadow_buffer_img;
    texture shadow_buffer_tex;
};

int scene_create(scene *scene, int scene_id);
void scene_init(scene *scene);
void scene_free(scene *scene);
int scene_event(scene *scene, SDL_Event *event);
void scene_render_overlay(scene *scene);
void scene_render(scene *scene);
void scene_render_shadows(scene *scene);
void scene_tick(scene *scene);
void scene_input_tick(scene *scene);
int scene_startup(scene *scene, int id);

void scene_set_userdata(scene *scene, void *userdata);
void* scene_get_userdata(scene *scene);

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc);
void scene_set_event_cb(scene *scene, scene_event_cb cbfunc);
void scene_set_render_cb(scene *scene, scene_render_cb cbfunc);
void scene_set_render_overlay_cb(scene *scene, scene_render_overlay_cb cbfunc);
void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc);
void scene_set_input_tick_cb(scene *scene, scene_input_tick_cb cbfunc);
void scene_set_startup_cb(scene *scene, scene_startup_cb cbfunc);

#endif // _SCENE_H
