#include "game/game_player.h"
#include "utils/allocator.h"
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
    // allocate the pilot separately so can be a pointer
    // into a CHR file in tournament mode
    gp->pilot = omf_calloc(1, sizeof(sd_pilot));
    sd_pilot_create(gp->pilot);
    chr_score_create(&gp->score);
    har_screencaps_create(&gp->screencaps);
    gp->god = 0;
    gp->ez_destruct = 0;
    gp->sp_wins = 0;
}

void game_player_free(game_player *gp) {
    sd_pilot_free(gp->pilot);
    chr_score_free(&gp->score);
    har_screencaps_free(&gp->screencaps);
}

void game_player_set_har(game_player *gp, object *har) {
    gp->har = har;
}

object *game_player_get_har(game_player *gp) {
    return gp->har;
}

void game_player_set_ctrl(game_player *gp, controller *ctrl) {
    if(gp->ctrl != NULL) {
        gp->ctrl->free_fun(gp->ctrl);
        omf_free(gp->ctrl);
    }
    gp->ctrl = ctrl;
}

controller *game_player_get_ctrl(game_player *gp) {
    return gp->ctrl;
}

void game_player_set_portrait(game_player *gp, surface *portrait) {
    gp->portrait = portrait;
}

surface *game_player_get_portrait(game_player *gp) {
    return gp->portrait;
}

void game_player_set_selectable(game_player *gp, int selectable) {
    gp->selectable = selectable;
}

int game_player_get_selectable(game_player *gp) {
    return gp->selectable;
}

sd_pilot *game_player_get_pilot(game_player *gp) {
    return gp->pilot;
}

void game_player_set_pilot(game_player *gp, sd_pilot *new_pilot) {
    gp->pilot = new_pilot;
}

chr_score *game_player_get_score(game_player *gp) {
    return &gp->score;
}
