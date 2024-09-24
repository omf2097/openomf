#include "game/protos/scene.h"
#include "video/video.h"

void lobby_input_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                   i->event_data.action == ACT_PUNCH) {
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

int lobby_create(scene *scene) {
    scene_set_input_poll_cb(scene, lobby_input_tick);

    video_render_bg_separately(false);
    return 0;
}
