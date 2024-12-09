#include "game/scenes/mainmenu.h"
#include "audio/audio.h"
#include "game/gui/frame.h"
#include "game/scenes/mainmenu/menu_main.h"
#include "game/scenes/mainmenu/menu_widget_ids.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/vga_state.h"
#include "video/video.h"
#include <SDL.h>

typedef struct mainmenu_local_t {
    guiframe *frame;
    int prev_key[2];
} mainmenu_local;

void mainmenu_free(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    guiframe_free(local->frame);
    omf_free(local);
    scene_set_userdata(scene, local);
    settings_save();
}

void mainmenu_tick(scene *scene, int paused) {
    mainmenu_local *local = scene_get_userdata(scene);
    palette_pulse_menu_colors(scene->gs->tick / 8);
    guiframe_tick(local->frame);
}

void mainmenu_input_tick(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(scene->gs, i);

        // Poll the controller
        ctrl_event *p = NULL, *orig_p = NULL;
        controller_poll(player->ctrl, &orig_p);
        p = orig_p;
        if(p) {
            do {
                if(p->type == EVENT_TYPE_ACTION) {
                    // Skip repeated keys
                    if(local->prev_key[i] == p->event_data.action) {
                        continue;
                    }

                    local->prev_key[i] = p->event_data.action;

                    // Pass on the event
                    guiframe_action(local->frame, p->event_data.action);
                }
            } while((p = p->next));
        }
        controller_free_chain(orig_p);
    }
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    mainmenu_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if(player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
       (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN &&
        keyboard_binds_key(player1->ctrl, event))) {
        // these events will be handled by polling
        return 1;
    }

    return guiframe_event(local->frame, event);
}

void mainmenu_render(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    guiframe_render(local->frame);
}

void mainmenu_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    switch(id) {
        case 10:
        case 11:
            *m_load = 1;
            *m_repeat = 1;
            break;
    }
}

// Init menus
int mainmenu_create(scene *scene) {
    // Init local data
    mainmenu_local *local = omf_calloc(1, sizeof(mainmenu_local));
    scene_set_userdata(scene, local);

    // fix up any jank from tournament mode
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);
    if(player1->chr) {
        sd_chr_free(player1->chr);
        omf_free(player1->chr);
        player1->pilot = omf_calloc(1, sizeof(sd_pilot));
        sd_pilot_create(player1->pilot);
    }

    if(!player2->pilot) {
        player2->pilot = omf_calloc(1, sizeof(sd_pilot));
        sd_pilot_create(player2->pilot);
    }

    // Load settings
    game_state_set_speed(scene->gs, settings_get()->gameplay.speed + 5);

    // Create main menu
    local->frame = guiframe_create(165, 5, 151, 119);
    guiframe_set_root(local->frame, menu_main_create(scene));
    guiframe_layout(local->frame);

    // Cleanups and resets
    for(int i = 0; i < 2; i++) {
        // destroy any leftover controllers
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_ctrl(player, NULL);

        // reset any single player data
        player->sp_wins = 0;
        chr_score *score = game_player_get_score(player);
        chr_score_reset(score, 1);
        chr_score_reset_wins(score);
    }
    reconfigure_controller(scene->gs);

    // Set callbacks
    scene_set_event_cb(scene, mainmenu_event);
    scene_set_input_poll_cb(scene, mainmenu_input_tick);
    scene_set_render_overlay_cb(scene, mainmenu_render);
    scene_set_free_cb(scene, mainmenu_free);
    scene_set_dynamic_tick_cb(scene, mainmenu_tick);
    scene_set_startup_cb(scene, mainmenu_startup);

    if(scene->gs->net_mode == NET_MODE_CLIENT) {
        component_action(guiframe_find(local->frame, NETWORK_BUTTON_ID), ACT_PUNCH);
        component_action(guiframe_find(local->frame, NETWORK_CONNECT_BUTTON_ID), ACT_PUNCH);
        component_action(guiframe_find(local->frame, NETWORK_CONNECT_IP_BUTTON_ID), ACT_PUNCH);
    } else if(scene->gs->net_mode == NET_MODE_SERVER) {
        component_action(guiframe_find(local->frame, NETWORK_BUTTON_ID), ACT_PUNCH);
        component_action(guiframe_find(local->frame, NETWORK_LISTEN_BUTTON_ID), ACT_PUNCH);
    } else if(scene->gs->net_mode == NET_MODE_LOBBY) {
        component_action(guiframe_find(local->frame, NETWORK_BUTTON_ID), ACT_PUNCH);
        component_action(guiframe_find(local->frame, NETWORK_LOBBY_BUTTON_ID), ACT_PUNCH);
    }

    // clear it, so this only happens the first time
    scene->gs->net_mode = NET_MODE_NONE;

    // prev_key is used to prevent multiple clicks while key is down
    local->prev_key[0] = local->prev_key[1] = ACT_PUNCH;

    // Music and renderer
    audio_play_music(PSM_MENU);

    // All done
    return 0;
}
