#include "game/game_player.h"

void game_player_create(game_player *gp) {
	gp->har_id = 0;
	gp->player_id = 0;
	gp->har = NULL;
	gp->ctrl = NULL;
	gp->portrait = NULL;
	gp->selectable = 0;
}

void game_player_free(game_player *gp) {

}

void game_player_set_har(game_player *gp, object *har) {
	if(hp->har != NULL) {
		har_free(hp->har);
		free(hp->har);
	}
	hp->har = har;
}

object* game_player_get_har(game_player *gp) {
	return gp->har;
}

void game_player_set_ctrl(game_player *gp, controller *ctrl) {
	if(gp->ctrl != NULL) {
		if(gp->ctrl->type == CTRL_TYPE_KEYBOARD) {
			keyboard_free(gp->ctrl);
		}
		free(gp->ctrl);
	}
	gp->ctrl = ctrl;
}

controller* game_player_get_ctrl(game_player *gp) {
	return gp->ctrl;
}

void game_player_set_portrait(game_player *gp, texture *portrait) {
	gp->portrait = portrait;
}

texture* game_player_get_portrait(game_player *gp) {
	return gp->portrait;
}

void game_player_set_selectable(game_player *gp, int selectable) {
	gp->selectable = selectable;
}

int game_player_get_selectable(game_player *gp) {
	return gp->selectable;
}