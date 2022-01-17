#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "controller/ai_controller.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "controller/net_controller.h"
#include "formats/pilot.h"
#include "game/protos/object.h"
#include "game/utils/har_screencap.h"
#include "game/utils/score.h"
#include "video/surface.h"

typedef struct game_player_t {
    int har_id;   // HAR_JAGUAR to HAR_NOVA
    int pilot_id; // 0 to 9
    object *har;
    controller *ctrl;
    surface *portrait;
    sd_pilot pilot;
    int selectable;
    char colors[3];
    chr_score score;
    int god;
    int ez_destruct;
    int sp_wins;
    har_screencaps screencaps;
} game_player;

void game_player_create(game_player *gp);
void game_player_free(game_player *gp);

void game_player_set_har(game_player *gp, object *har);
object *game_player_get_har(game_player *gp);
void game_player_set_ctrl(game_player *gp, controller *ctrl);
controller *game_player_get_ctrl(game_player *gp);
void game_player_set_portrait(game_player *gp, surface *portrait);
surface *game_player_get_portrait(game_player *gp);
void game_player_set_selectable(game_player *gp, int selectable);
int game_player_get_selectable(game_player *gp);
sd_pilot *game_player_get_pilot(game_player *gp);
chr_score *game_player_get_score(game_player *gp);

#endif // GAME_PLAYER_H
