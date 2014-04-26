#include "game/game_player.h"
#include <stdlib.h>

void game_player_create(game_player *gp) {
    gp->har_id = 0;
    gp->pilot_id = 0;
    gp->har = NULL;
    gp->ctrl = NULL;
    gp->portrait = NULL;
    gp->selectable = 0;
    // default to grey
    gp->colors[0] = 8;
    gp->colors[1] = 8;
    gp->colors[2] = 8;
    chr_score_create(&gp->score, 1.0f);
    har_screencaps_create(&gp->screencaps);
    gp->god = 0;
    gp->ez_destruct = 0;
    gp->sp_wins = 0;
}

void game_player_free(game_player *gp) {
    chr_score_free(&gp->score);
    har_screencaps_free(&gp->screencaps);
}

void game_player_set_har(game_player *gp, object *har) {
    gp->har = har;
}

object* game_player_get_har(game_player *gp) {
    return gp->har;
}

void game_player_set_ctrl(game_player *gp, controller *ctrl) {
    if(gp->ctrl != NULL) {
        if(gp->ctrl->type == CTRL_TYPE_KEYBOARD) {
            keyboard_free(gp->ctrl);
        } else if(gp->ctrl->type == CTRL_TYPE_NETWORK) {
            net_controller_free(gp->ctrl);
        } else if(gp->ctrl->type == CTRL_TYPE_AI) {
            ai_controller_free(gp->ctrl);
        }
        free(gp->ctrl);
    }
    gp->ctrl = ctrl;
}

controller* game_player_get_ctrl(game_player *gp) {
    return gp->ctrl;
}

void game_player_set_portrait(game_player *gp, surface *portrait) {
    gp->portrait = portrait;
}

surface* game_player_get_portrait(game_player *gp) {
    return gp->portrait;
}

void game_player_set_selectable(game_player *gp, int selectable) {
    gp->selectable = selectable;
}

int game_player_get_selectable(game_player *gp) {
    return gp->selectable;
}

chr_score* game_player_get_score(game_player *gp) {
    return &gp->score;
}
