#ifndef _GAME_PLAYER_H
#define _GAME_PLAYER_H

#include "game/protos/object.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "controller/net_controller.h"
#include "video/surface.h"

typedef struct game_player_t {
    int har_id; // HAR_JAGUAR to HAR_NOVA
    int pilot_id; // 0 to 9
    object *har;
    controller *ctrl;
    surface *portrait;
    int selectable;
    char colors[3];
} game_player;

void game_player_create(game_player *gp);
void game_player_free(game_player *gp);

void game_player_set_har(game_player *gp, object *har);
object* game_player_get_har(game_player *gp);
void game_player_set_ctrl(game_player *gp, controller *ctrl);
controller* game_player_get_ctrl(game_player *gp);
void game_player_set_portrait(game_player *gp, surface *portrait);
surface* game_player_get_portrait(game_player *gp);
void game_player_set_selectable(game_player *gp, int selectable);
int game_player_get_selectable(game_player *gp);

#endif // _GAME_PLAYER_H
