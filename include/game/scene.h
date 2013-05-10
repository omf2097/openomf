#ifndef _SCENE_H
#define _SCENE_H

#include <SDL2/SDL.h>
#include "video/texture.h"
#include "utils/array.h"
#include "utils/list.h"
#include "game/har.h"
#include "controller/controller.h"

enum {
    SCENE_INTRO = 0,
    SCENE_MENU,
    SCENE_END,
    SCENE_END1,
    SCENE_END2,
    SCENE_CREDITS,
    SCENE_MECHLAB,
    SCENE_VS,
    SCENE_MELEE,
    SCENE_NEWSROOM,
    SCENE_TOURNAMENT,
    SCENE_ARENA0,
    SCENE_ARENA1,
    SCENE_ARENA2,
    SCENE_ARENA3,
    SCENE_ARENA4,
    SCENE_ARENA5,
    SCENE_NORTHAM,
    SCENE_KATUSHAI,
    SCENE_WAR,
    SCENE_WORLD,
    SCENE_NONE
};

typedef struct scene_player_t {
    int har_id;
    int player_id;
    har *har;
    controller *ctrl;
    texture *portrait;
    int selectable;
    
    const char *player_name;
    const char *har_name;
    
    // store crap like agility and stuff here?
} scene_player;

typedef struct scene_t {
    struct sd_bk_file_t *bk;
    unsigned int loop;
    unsigned int this_id;
    unsigned int next_id;
    texture background;
    array animations;

    list child_players;
    list root_players;

    scene_player player1;
    scene_player player2;

    int (*init)(struct scene_t *scene);
    void (*post_init)(struct scene_t *scene);
    int (*event)(struct scene_t *scene, SDL_Event *event);
    void (*render)(struct scene_t *scene);
    void (*deinit)(struct scene_t *scene);
    void (*tick)(struct scene_t *scene);
} scene;

void scene_set_player1_har(scene *scene, har *har);
void scene_set_player2_har(scene *scene, har *har);
void scene_set_player1_ctrl(scene *scene, controller *ctrl);
void scene_set_player2_ctrl(scene *scene, controller *ctrl);

int scene_is_valid(int id);
int scene_load(scene *scene, unsigned int scene_id);
void scene_free(scene *scene);
int scene_handle_event(scene *scene, SDL_Event *event);
void scene_render(scene *scene);
void scene_tick(scene *scene);

#endif // _SCENE_H
