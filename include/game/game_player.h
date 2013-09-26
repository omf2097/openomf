#ifndef _GAME_PLAYER_H
#define _GAME_PLAYER_H

#include "game/protos/object.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "video/texture.h"

typedef struct game_player_t {
    int har_id;
    int player_id;
    object *har;
    controller *ctrl;
    texture *portrait;
    int selectable;
} game_player;

void game_player_create(game_player *gp);
void game_player_free(game_player *gp);

void game_player_set_har(game_player *gp, object *har);
object* game_player_get_har(game_player *gp);
void game_player_set_ctrl(game_player *gp, controller *ctrl);
controller* game_player_get_ctrl(game_player *gp);
void game_player_set_portrait(game_player *gp, texture *portrait);
texture* game_player_get_portrait(game_player *gp);
void game_player_set_selectable(game_player *gp, int selectable);
int game_player_get_selectable(game_player *gp);

#endif // _GAME_PLAYER_H
