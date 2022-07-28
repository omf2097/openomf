#include <SDL.h>
#include <stdlib.h>

#include "formats/error.h"
#include "game/game_state.h"
#include "game/gui/frame.h"
#include "game/gui/trn_menu.h"
#include "game/protos/object.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/lab_dash_main.h"
#include "game/scenes/mechlab/lab_dash_newplayer.h"
#include "game/scenes/mechlab/lab_menu_main.h"
#include "game/scenes/mechlab/lab_menu_pilotselect.h"
#include "game/utils/settings.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct {
    dashboard_type dash_type;
    object bg_obj[3];
    guiframe *frame;
    guiframe *dashboard;
    object *mech;
    dashboard_widgets dw;
    newplayer_widgets nw;
} mechlab_local;

void mechlab_free(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj) / sizeof(object); i++) {
        object_free(&local->bg_obj[i]);
    }

    guiframe_free(local->frame);
    guiframe_free(local->dashboard);
    object_free(local->mech);
    omf_free(local->mech);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void mechlab_tick(scene *scene, int paused) {
    mechlab_local *local = scene_get_userdata(scene);

    guiframe_tick(local->frame);
    guiframe_tick(local->dashboard);
    object_dynamic_tick(local->mech);

    // Check if root is finished
    component *root = guiframe_get_root(local->frame);
    if(trnmenu_is_finished(root)) {
        if(local->dash_type == DASHBOARD_SELECT_NEW_PIC) {
            guiframe_free(local->frame);
            local->frame = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->frame, lab_menu_pilotselect_create(scene, &local->dw));
            guiframe_layout(local->frame);
        } else {
            game_state_set_next(scene->gs, SCENE_MENU);
        }
    }
}

void mechlab_select_dashboard(scene *scene, dashboard_type type) {
    mechlab_local *local = scene_get_userdata(scene);
    if(type == local->dash_type) {
        // No change
        return;
    }

    // Free old dashboard if set
    if(local->dashboard != NULL) {
        guiframe_free(local->dashboard);
    }

    // Switch to new dashboard
    local->dash_type = type;
    switch(type) {
        // Dashboard with the gauges etc.
        case DASHBOARD_STATS:
        case DASHBOARD_SELECT_NEW_PIC:
            // Dashboard widgets struct is filled with pointer to the necessary components for easy access
            local->dashboard = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->dashboard, lab_dash_main_create(scene, &local->dw));
            lab_dash_main_update(scene, &local->dw);
            guiframe_layout(local->dashboard);
            break;
        // Dashboard for new player
        case DASHBOARD_NEW:
            local->dashboard = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->dashboard, lab_dash_newplayer_create(scene, &local->nw));
            guiframe_layout(local->dashboard);
            break;
        // No dashboard selection. This shouldn't EVER happen.
        case DASHBOARD_NONE:
            PERROR("No dashboard selected; this should not happen!");
            local->dashboard = NULL;
            break;
    }
}

int mechlab_event(scene *scene, SDL_Event *event) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if(player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
       (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN &&
        keyboard_binds_key(player1->ctrl, event))) {
        // these events will be handled by polling
        return 1;
    }

    if(local->dash_type == DASHBOARD_NEW) {
        return guiframe_event(local->dashboard, event);
    } else {
        return guiframe_event(local->frame, event);
    }
}

void mechlab_render(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj) / sizeof(object); i++) {
        object_render(&local->bg_obj[i]);
    }

    // Render dashboard
    guiframe_render(local->frame);
    guiframe_render(local->dashboard);

    // Only render mech in stats dashboard
    if(local->dash_type == DASHBOARD_STATS) {
        object_render(local->mech);
    }
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
                // If view is new dashboard view, pass all input to it
                if(local->dash_type == DASHBOARD_NEW) {
                    // If inputting text for new player name is done, switch to next view.
                    // If ESC, exit view.
                    // Otherwise, handle text input
                    if(i->event_data.action == ACT_ESC) {
                        trnmenu_finish(guiframe_get_root(local->frame));
                    } else if(i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH) {
                        mechlab_select_dashboard(scene, DASHBOARD_SELECT_NEW_PIC);
                        trnmenu_finish(
                            guiframe_get_root(local->frame)); // This will trigger exception case in mechlab_tick
                    } else {
                        guiframe_action(local->dashboard, i->event_data.action);
                    }
                    // If view is any other, just pass input to the bottom menu
                } else {
                    guiframe_action(local->frame, i->event_data.action);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

// Init mechlab
int mechlab_create(scene *scene) {
    // Alloc
    mechlab_local *local = omf_calloc(1, sizeof(mechlab_local));

    animation *bg_ani[3];

    // Init the background
    for(int i = 0; i < sizeof(bg_ani) / sizeof(animation *); i++) {
        sprite *spr = sprite_copy(animation_get_sprite(&bk_get_info(&scene->bk_data, 14)->ani, i));
        bg_ani[i] = create_animation_from_single(spr, spr->pos);
        object_create(&local->bg_obj[i], scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(&local->bg_obj[i], bg_ani[i]);
        object_select_sprite(&local->bg_obj[i], 0);
        object_set_repeat(&local->bg_obj[i], 1);
        object_set_animation_owner(&local->bg_obj[i], OWNER_OBJECT);
    }

    // Find last saved game ...
    game_player *p1 = game_state_get_player(scene->gs, 0);
    const char *last_name = settings_get()->tournament.last_name;
    if(last_name == NULL || strlen(last_name) == 0) {
        last_name = NULL;
    }

    // ... and attempt to load it, if one was found.
    if(last_name != NULL) {
        int ret = sg_load(&p1->pilot, last_name);
        if(ret != SD_SUCCESS) {
            PERROR("Could not load saved game for player name '%s': %s!", last_name, sd_get_error(ret));
            last_name = NULL;
        } else {
            DEBUG("Loaded saved game for player name '%s'.", last_name);
        }
    }

    // Either initialize a new tournament if no saved game is found,
    // or just show old saved game stats directly if it was.
    local->dash_type = DASHBOARD_NONE;
    if(last_name == NULL) {
        DEBUG("No previous saved game found");
    } else {
        DEBUG("Previous saved game found; loading as default.");
    }

    // Create main menu
    local->frame = guiframe_create(0, 0, 320, 200);
    guiframe_set_root(local->frame, lab_menu_main_create(scene));
    guiframe_layout(local->frame);

    // Load HAR
    animation *initial_har_ani = &bk_get_info(&scene->bk_data, 15 + p1->pilot.har_id)->ani;
    local->mech = omf_calloc(1, sizeof(object));
    object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
    object_set_animation(local->mech, initial_har_ani);
    object_set_repeat(local->mech, 1);
    object_dynamic_tick(local->mech);

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_input_poll_cb(scene, mechlab_input_tick);
    scene_set_event_cb(scene, mechlab_event);
    scene_set_render_cb(scene, mechlab_render);
    scene_set_free_cb(scene, mechlab_free);
    scene_set_dynamic_tick_cb(scene, mechlab_tick);

    // Don't render background on its own layer
    // Fix for some additive blending tricks.
    video_render_bg_separately(false);

    // Pick the initial dashboard page. Note that userdata must be set for scene!
    mechlab_select_dashboard(scene, DASHBOARD_STATS);
    return 0;
}
