#include <stdlib.h>
#include <SDL2/SDL.h>

#include "game/protos/object.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/lab_main.h"
#include "game/gui/frame.h"
#include "game/protos/scene.h"
#include "game/game_state.h"
#include "video/video.h"
#include "utils/log.h"
#include "resources/ids.h"

typedef struct {
    object bg_obj[3];
    guiframe *frame;
} mechlab_local;

void mechlab_free(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj)/sizeof(object); i++) {
        object_free(&local->bg_obj[i]);
    }

    guiframe_free(local->frame);
    free(local);
}

void mechlab_tick(scene *scene, int paused) {
    mechlab_local *local = scene_get_userdata(scene);

    guiframe_tick(local->frame);
}

int mechlab_event(scene *scene, SDL_Event *event) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if (player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
            (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN
             && keyboard_binds_key(player1->ctrl, event))) {
        // these events will be handled by polling
        return 1;
    }

    return guiframe_event(local->frame, event);
}

void mechlab_render(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj)/sizeof(object); i++) {
        object_render(&local->bg_obj[i]);
    }

    guiframe_render(local->frame);
}

void mechlab_input_tick(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    // Poll the controller
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                guiframe_action(local->frame, i->event_data.action);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

// Init mechlab
int mechlab_create(scene *scene) {
    // Alloc
    mechlab_local *local = malloc(sizeof(mechlab_local));
    memset(local, 0, sizeof(mechlab_local));

    animation *bg_ani[3];

    // Init the background
    for(int i = 0; i < sizeof(bg_ani)/sizeof(animation*); i++) {
        sprite *spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 14)->ani, i));
        bg_ani[i] = create_animation_from_single(spr, spr->pos);
        object_create(&local->bg_obj[i], scene->gs, vec2i_create(0,0), vec2f_create(0,0));
        object_set_animation(&local->bg_obj[i], bg_ani[i]);
        object_select_sprite(&local->bg_obj[i], 0);
        object_set_repeat(&local->bg_obj[i], 1);
        object_set_animation_owner(&local->bg_obj[i], OWNER_OBJECT);
    }

    // Create main menu
    local->frame = guiframe_create(0, 0, 320, 200);
    guiframe_set_root(local->frame, lab_main_create(scene));
    guiframe_layout(local->frame);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_input_poll_cb(scene, mechlab_input_tick);
    scene_set_event_cb(scene, mechlab_event);
    scene_set_render_cb(scene, mechlab_render);
    scene_set_free_cb(scene, mechlab_free);
    scene_set_dynamic_tick_cb(scene, mechlab_tick);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_HW);

    return 0;
}
