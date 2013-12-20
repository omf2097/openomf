#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include "controller/controller.h"
#include "game/protos/object.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/serial.h"
#include "resources/bk.h"

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct game_state_t game_state;

typedef void (*scene_free_cb)(scene *scene);
typedef int (*scene_event_cb)(scene *scene, SDL_Event *event);
typedef void (*scene_render_cb)(scene *scene);
typedef void (*scene_render_overlay_cb)(scene *scene);
typedef void (*scene_tick_cb)(scene *scene);
typedef void (*scene_input_poll_cb)(scene *scene);
typedef int (*scene_startup_cb)(scene *scene, int anim_id);
typedef int (*scene_serialize_cb)(scene *scene, serial *ser);

struct scene_t {
    game_state *gs;
    int id;
    bk bk_data;
    void *userdata;
    object background;
    scene_free_cb free;
    scene_event_cb event;
    scene_render_cb render;
    scene_render_overlay_cb render_overlay;
    scene_tick_cb tick;
    scene_input_poll_cb input_poll;
    scene_startup_cb startup;
    scene_serialize_cb serialize;
    image shadow_buffer_img;
    texture shadow_buffer_tex;
};

int scene_create(scene *scene, game_state *gs, int scene_id);
void scene_init(scene *scene);
void scene_free(scene *scene);
int scene_event(scene *scene, SDL_Event *event);
void scene_render_overlay(scene *scene);
void scene_render(scene *scene);
void scene_render_shadows(scene *scene);
void scene_tick(scene *scene);
void scene_input_poll(scene *scene);
int scene_startup(scene *scene, int id);

int scene_serialize(scene *scene, serial *ser);
int scene_unserialize(scene *scene, serial *ser);

void scene_set_userdata(scene *scene, void *userdata);
void* scene_get_userdata(scene *scene);

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc);
void scene_set_event_cb(scene *scene, scene_event_cb cbfunc);
void scene_set_render_cb(scene *scene, scene_render_cb cbfunc);
void scene_set_render_overlay_cb(scene *scene, scene_render_overlay_cb cbfunc);
void scene_set_tick_cb(scene *scene, scene_tick_cb cbfunc);
void scene_set_input_poll_cb(scene *scene, scene_input_poll_cb cbfunc);
void scene_set_startup_cb(scene *scene, scene_startup_cb cbfunc);
void scene_set_serialize_cb(scene *scene, scene_serialize_cb cbfunc);

#endif // _SCENE_H
