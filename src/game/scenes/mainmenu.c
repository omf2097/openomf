/*
#include <enet/enet.h>
#include <time.h>
#include <stdio.h>
#include "engine.h"
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/audio.h"
#include "game/common_defines.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "controller/joystick.h"
#include "controller/net_controller.h"
#include "resources/ids.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "plugins/plugins.h"
*/

#include <SDL2/SDL.h>
#include "audio/music.h"
#include "video/video.h"
#include "resources/ids.h"
#include "game/menu/frame.h"
#include "game/scenes/mainmenu.h"
#include "game/scenes/mainmenu/menu_main.h"
#include "game/utils/settings.h"

typedef struct mainmenu_local_t {
/*
    time_t connect_start;
    char input_key_labels[6][100];
    int input_presskey_ready_ticks;
    int input_selected_player;
*/

    guiframe *frame;
    int prev_key;
} mainmenu_local;

/*
void update_keys(mainmenu_local *local, int player) {
    settings_keyboard *k = &settings_get()->keys;
    if(player == 1) {
        sprintf(local->input_key_labels[0], "UP: %s", k->key1_up);
        sprintf(local->input_key_labels[1], "DOWN: %s", k->key1_down);
        sprintf(local->input_key_labels[2], "LEFT: %s", k->key1_left);
        sprintf(local->input_key_labels[3], "RIGHT: %s", k->key1_right);
        sprintf(local->input_key_labels[4], "PUNCH: %s", k->key1_punch);
        sprintf(local->input_key_labels[5], "KICK: %s", k->key1_kick);
    } else {
        sprintf(local->input_key_labels[0], "UP: %s", k->key2_up);
        sprintf(local->input_key_labels[1], "DOWN: %s", k->key2_down);
        sprintf(local->input_key_labels[2], "LEFT: %s", k->key2_left);
        sprintf(local->input_key_labels[3], "RIGHT: %s", k->key2_right);
        sprintf(local->input_key_labels[4], "PUNCH: %s", k->key2_punch);
        sprintf(local->input_key_labels[5], "KICK: %s", k->key2_kick);
    }
}

void mainmenu_set_right_keyboard(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_keyboard *k = &settings_get()->keys;
    if(local->input_selected_player == 1) {
        free(k->key1_up);
        k->key1_up = dupestr(SDL_GetScancodeName(SDL_SCANCODE_UP));
        free(k->key1_down);
        k->key1_down = dupestr(SDL_GetScancodeName(SDL_SCANCODE_DOWN));
        free(k->key1_left);
        k->key1_left = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LEFT));
        free(k->key1_right);
        k->key1_right = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RIGHT));
        free(k->key1_punch);
        k->key1_punch = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RETURN));
        free(k->key1_kick);
        k->key1_kick = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RSHIFT));
        update_keys(local, 1);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene*) userdata)->gs);
    } else  if(local->input_selected_player == 2) {
        free(k->key2_up);
        k->key2_up = dupestr(SDL_GetScancodeName(SDL_SCANCODE_UP));
        free(k->key2_down);
        k->key2_down = dupestr(SDL_GetScancodeName(SDL_SCANCODE_DOWN));
        free(k->key2_left);
        k->key2_left = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LEFT));
        free(k->key2_right);
        k->key2_right = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RIGHT));
        free(k->key2_punch);
        k->key2_punch = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RETURN));
        free(k->key2_kick);
        k->key2_kick = dupestr(SDL_GetScancodeName(SDL_SCANCODE_RSHIFT));
        update_keys(local, 2);
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
    }
}

void mainmenu_set_left_keyboard(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_keyboard *k = &settings_get()->keys;
    if(local->input_selected_player == 1) {
        free(k->key1_up);
        k->key1_up = dupestr(SDL_GetScancodeName(SDL_SCANCODE_W));
        free(k->key1_down);
        k->key1_down = dupestr(SDL_GetScancodeName(SDL_SCANCODE_S));
        free(k->key1_left);
        k->key1_left = dupestr(SDL_GetScancodeName(SDL_SCANCODE_A));
        free(k->key1_right);
        k->key1_right = dupestr(SDL_GetScancodeName(SDL_SCANCODE_D));
        free(k->key1_punch);
        k->key1_punch = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LSHIFT));
        free(k->key1_kick);
        k->key1_kick = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LCTRL));
        update_keys(local, 1);
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
        reconfigure_controller(((scene*) userdata)->gs);
    } else  if(local->input_selected_player == 2) {
        free(k->key2_up);
        k->key2_up = dupestr(SDL_GetScancodeName(SDL_SCANCODE_W));
        free(k->key2_down);
        k->key2_down = dupestr(SDL_GetScancodeName(SDL_SCANCODE_S));
        free(k->key2_left);
        k->key2_left = dupestr(SDL_GetScancodeName(SDL_SCANCODE_A));
        free(k->key2_right);
        k->key2_right = dupestr(SDL_GetScancodeName(SDL_SCANCODE_D));
        free(k->key2_punch);
        k->key2_punch = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LSHIFT));
        free(k->key2_kick);
        k->key2_kick = dupestr(SDL_GetScancodeName(SDL_SCANCODE_LCTRL));
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
        update_keys(local, 2);
    }
}

void mainmenu_set_joystick1(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_keyboard *k = &settings_get()->keys;
    if(local->input_selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name1);
        k->joy_name1 = dupestr(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset1 = joystick_offset(joystick_nth_id(1), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name2);
        k->joy_name2 = dupestr(SDL_JoystickNameForIndex(joystick_nth_id(1)));
        k->joy_offset2 = joystick_offset(joystick_nth_id(1), k->joy_name2);
    }
    reconfigure_controller(((scene*) userdata)->gs);
}

void mainmenu_set_joystick2(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_keyboard *k = &settings_get()->keys;
    if(local->input_selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name1);
        k->joy_name1 = dupestr(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset1= joystick_offset(joystick_nth_id(2), k->joy_name1);
    } else {
        k->ctrl_type2 = CTRL_TYPE_GAMEPAD;
        free(k->joy_name2);
        k->joy_name2 = dupestr(SDL_GameControllerNameForIndex(joystick_nth_id(2)));
        k->joy_offset2= joystick_offset(joystick_nth_id(2), k->joy_name2);
    }
    reconfigure_controller(((scene*) userdata)->gs);
}

void mainmenu_apply_custom_input_config(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_keyboard *k = &settings_get()->keys;
    if(local->input_selected_player == 1) {
        k->ctrl_type1 = CTRL_TYPE_KEYBOARD;
    } else {
        k->ctrl_type2 = CTRL_TYPE_KEYBOARD;
    }
    reconfigure_controller(((scene*) userdata)->gs);
    mainmenu_prev_menu(c, userdata);
}

void mainmenu_enter_playerone_input_config(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->input_config_menu;
    local->current_menu = &local->input_config_menu;
    update_keys(local, 1);
    local->input_selected_player = 1;
}

void mainmenu_enter_playertwo_input_config(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->input_config_menu;
    local->current_menu = &local->input_config_menu;
    update_keys(local, 2);
    local->input_selected_player = 2;
}

void inputmenu_set_key(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    //settings_keyboard *k = &settings_get()->keys;
    local->mstack[local->mstack_pos++] = &local->input_presskey_menu;
    local->current_menu = &local->input_presskey_menu;
    local->input_presskey_ready_ticks = 0;
}


*/

void mainmenu_free(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    guiframe_free(local->frame);
    free(local);
    settings_save();
}

void mainmenu_tick(scene *scene, int paused) {
    mainmenu_local *local = scene_get_userdata(scene);
    //game_state *gs = scene->gs;

    // Tick menu
    guiframe_tick(local->frame);

/*

    if(local->mstack[local->mstack_pos-1] == &local->input_presskey_menu) {
        if(local->input_presskey_ready_ticks == 20) {
            int keys = 0;
            const unsigned char *state = SDL_GetKeyboardState(&keys);
            for(int i = 0;i < keys; i++) {
                if(state[i]) {
                    settings_keyboard *k = &settings_get()->keys;
                    component *selected = *(component**)vector_get(&local->input_custom_keyboard_menu.objs, local->input_custom_keyboard_menu.selected);
                    char *keyname = dupestr(SDL_GetScancodeName(i));
                    if(local->input_selected_player == 1) {
                        if(selected == &local->input_up_button) {
                            free(k->key1_up);
                            k->key1_up = keyname;
                        } else if(selected == &local->input_down_button) {
                            free(k->key1_down);
                            k->key1_down = keyname;
                        } else if(selected == &local->input_left_button) {
                            free(k->key1_left);
                            k->key1_left = keyname;
                        } else if(selected == &local->input_right_button) {
                            free(k->key1_right);
                            k->key1_right = keyname;
                        } else if(selected == &local->input_punch_button) {
                            free(k->key1_punch);
                            k->key1_punch = keyname;
                        } else if(selected == &local->input_kick_button) {
                            free(k->key1_kick);
                            k->key1_kick = keyname;
                        } else {
                            free(keyname);
                        }
                        update_keys(local, 1);
                    } else if(local->input_selected_player == 2) {
                        if(selected == &local->input_up_button) {
                            free(k->key2_up);
                            k->key2_up = keyname;
                        } else if(selected == &local->input_down_button) {
                            free(k->key2_down);
                            k->key2_down = keyname;
                        } else if(selected == &local->input_left_button) {
                            free(k->key2_left);
                            k->key2_left = keyname;
                        } else if(selected == &local->input_right_button) {
                            free(k->key2_right);
                            k->key2_right = keyname;
                        } else if(selected == &local->input_punch_button) {
                            free(k->key2_punch);
                            k->key2_punch = keyname;
                        } else if(selected == &local->input_kick_button) {
                            free(k->key2_kick);
                            k->key2_kick = keyname;
                        } else {
                            free(keyname);
                        }
                        update_keys(local, 2);
                    } else {
                        free(keyname);
                    }
                    mainmenu_prev_menu(&local->input_presskey_header, scene);
                    break;
                }
            }
        } else {
            local->input_presskey_ready_ticks++;
        }
    }

*/
}


void mainmenu_input_tick(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    // Poll the controller
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                // Skip repeated keys
                if(local->prev_key == i->event_data.action) {
                    continue;
                }
                local->prev_key = i->event_data.action;

                // Pass on the event
                guiframe_action(local->frame, i->event_data.action);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    mainmenu_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if (player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
            (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN
             && keyboard_binds_key(player1->ctrl, event))) {
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
    mainmenu_local *local = malloc(sizeof(mainmenu_local));
    scene_set_userdata(scene, local);

    // Load settings
    game_state_set_speed(scene->gs, settings_get()->gameplay.speed);

    // Create main menu
    local->frame = guiframe_create(165, 5, 151, 119);
    guiframe_set_root(local->frame, menu_main_create(scene));
    guiframe_layout(local->frame);

    // Cleanups and resets
    for(int i = 0; i < 2; i++) {
        // destroy any leftover controllers
        controller *ctrl;
        if((ctrl = game_player_get_ctrl(game_state_get_player(scene->gs, i)))) {
            game_player_set_ctrl(game_state_get_player(scene->gs, i), NULL);
        }

        // reset any single player data
        game_state_get_player(scene->gs, i)->sp_wins = 0;
        chr_score_reset(game_player_get_score(game_state_get_player(scene->gs, i)), 1);
        chr_score_reset_wins(game_player_get_score(game_state_get_player(scene->gs, i)));
    }
    reconfigure_controller(scene->gs);

    // Set callbacks
    scene_set_event_cb(scene, mainmenu_event);
    scene_set_input_poll_cb(scene, mainmenu_input_tick);
    scene_set_render_overlay_cb(scene, mainmenu_render);
    scene_set_free_cb(scene, mainmenu_free);
    scene_set_dynamic_tick_cb(scene, mainmenu_tick);
    scene_set_startup_cb(scene, mainmenu_startup);

/*
    if(scene->gs->net_mode == NET_MODE_CLIENT) {
        component_click(&local->net_button);
        component_click(&local->net_connect_button);
        component_click(&local->connect_ip_button);
    } else if(scene->gs->net_mode == NET_MODE_SERVER) {
        component_click(&local->net_button);
        component_click(&local->net_listen_button);
    }


*/
    // clear it, so this only happens the first time
    scene->gs->net_mode = NET_MODE_NONE;

    // prev_key is used to prevent multiple clicks while key is down
    local->prev_key = ACT_PUNCH;

    // Music and renderer
    music_play(PSM_MENU);
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done
    return 0;
}
