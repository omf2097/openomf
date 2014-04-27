#include "video/video.h"
#include "video/surface.h"
#include "resources/ids.h"
#include "utils/log.h"
#include "game/scenes/scoreboard.h"

typedef struct scoreboard_local_t {
    surface black_surface;
} scoreboard_local;

void scoreboard_free(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    surface_free(&local->black_surface);
    free(local);
}

int scoreboard_event(scene *scene, SDL_Event *event) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC ||
                    i->event_data.action == ACT_KICK ||
                    i->event_data.action == ACT_PUNCH) {

                    game_state_set_next(scene->gs, SCENE_MENU);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
    return 1;
}

void scoreboard_render_overlay(scene *scene) {
    scoreboard_local *local = scene_get_userdata(scene);
    video_render_sprite_size(&local->black_surface, 0, 0, 320, 200);
    DEBUG("RENDER OVERLAY");
}

int scoreboard_create(scene *scene) {
    // Init local data
    scoreboard_local *local = malloc(sizeof(scoreboard_local));
    scene_set_userdata(scene, local);

    // Create a surface that has an appropriate alpha for darkening the screen a bit
    surface_create(&local->black_surface, SURFACE_TYPE_RGBA, 32, 32);
    surface_fill(&local->black_surface, color_create(0,0,0,200));

    // Set callbacks
    scene_set_event_cb(scene, scoreboard_event);
    scene_set_render_overlay_cb(scene, scoreboard_render_overlay);
    scene_set_free_cb(scene, scoreboard_free);
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done
    return 0;
}
