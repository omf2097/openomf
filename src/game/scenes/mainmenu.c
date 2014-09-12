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
#include "game/menu/menu.h"
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

    menu main_menu;
    int prev_key;
} mainmenu_local;

/*
char *dupestr(const char *s) {
    return strcpy(malloc(strlen(s)+1), s);
}
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

// Menu event handlers
void mainmenu_quit(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_CREDITS);
}

void mainmenu_1v1(component *c, void *userdata) {
    scene *s = userdata;

    // Set up controllers
    settings_keyboard *k = &settings_get()->keys;
    if (k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 0);
    } else if (k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 0, k->joy_name1, k->joy_offset1);
    }

    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 0)), settings_get()->gameplay.difficulty);
    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 1)), settings_get()->gameplay.difficulty);

    _setup_ai(s->gs, 1);

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_1v2(component *c, void *userdata) {
    scene *s = userdata;

    settings_keyboard *k = &settings_get()->keys;
    if (k->ctrl_type1 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 0);
    } else if (k->ctrl_type1 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 0, k->joy_name1, k->joy_offset1);
    }


    if (k->ctrl_type2 == CTRL_TYPE_KEYBOARD) {
        _setup_keyboard(s->gs, 1);
    } else if (k->ctrl_type2 == CTRL_TYPE_GAMEPAD) {
        _setup_joystick(s->gs, 1, k->joy_name2, k->joy_offset2);
    }

    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 0)), AI_DIFFICULTY_CHAMPION);
    chr_score_set_difficulty(game_player_get_score(game_state_get_player(s->gs, 1)), AI_DIFFICULTY_CHAMPION);

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_tourn(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_MECHLAB);
}

void mainmenu_demo(component *c, void *userdata) {
    scene *s = userdata;

    // Set up controllers
    game_state_init_demo(s->gs);

    game_state_set_next(s->gs, rand_arena());
}

void mainmenu_soreboard(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_SCOREBOARD);
}

void mainmenu_prev_menu(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[--local->mstack_pos] = NULL;
    local->current_menu = local->mstack[local->mstack_pos - 1];
}

void mainmenu_enter_menu_config(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->config_menu;
    local->current_menu = &local->config_menu;
}

void mainmenu_enter_menu_gameplay(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->gameplay_menu;
    local->current_menu = &local->gameplay_menu;
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

void mainmenu_enter_custom_keyboard_config(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->input_custom_keyboard_menu;
    local->current_menu = &local->input_custom_keyboard_menu;
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

void mainmenu_enter_menu_video(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->video_menu;
    local->current_menu = &local->video_menu;
    local->old_video_settings = settings_get()->video;
}

void mainmenu_enter_menu_video_confirm(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->video_confirm_menu;
    local->current_menu = &local->video_confirm_menu;
}

void mainmenu_enter_menu_net(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->net_menu;
    local->current_menu = &local->net_menu;
}

void mainmenu_enter_menu_connect(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->connect_menu;
    local->current_menu = &local->connect_menu;
}

void mainmenu_enter_menu_listen(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[local->mstack_pos++] = &local->listen_menu;
    local->current_menu = &local->listen_menu;
}

void inputmenu_set_key(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    //settings_keyboard *k = &settings_get()->keys;
    local->mstack[local->mstack_pos++] = &local->input_presskey_menu;
    local->current_menu = &local->input_presskey_menu;
    local->input_presskey_ready_ticks = 0;
}

void menu_music_slide(component *c, void *userdata, int pos) {
    music_set_volume(pos/10.0f);
}

void menu_sound_slide(component *c, void *userdata, int pos) {
    sound_set_volume(pos/10.0f);
}

void menu_mono_toggle(component *c, void *userdata, int options) {
    music_reload();
}

void menu_speed_slide(component *c, void *userdata, int pos) {
    scene *sc = userdata;
    game_state_set_speed(sc->gs, pos);
}

void mainmenu_connect_to_ip(component *c, void *userdata) {
    scene *s = (scene*)userdata;
    mainmenu_local *local = scene_get_userdata(s);
    ENetAddress address;
    char *addr = textinput_value(&local->connect_ip_input);
    free(settings_get()->net.net_connect_ip);
    settings_get()->net.net_connect_ip = dupestr(addr);
    local->host = enet_host_create(NULL, 1, 2, 0, 0);
    s->gs->role = ROLE_CLIENT;
    if (local->host == NULL) {
        DEBUG("Failed to initialize ENet client");
        return;
    }
    local->connect_ip_input.disabled = 1;
    local->connect_ip_button.disabled = 1;
    menu_select(&local->connect_menu, &local->connect_ip_cancel_button);

    enet_address_set_host(&address, addr);
    address.port = settings_get()->net.net_connect_port;

    ENetPeer *peer = enet_host_connect(local->host, &address, 2, 0);

    if (peer == NULL) {
        DEBUG("Unable to connect to %s", addr);
        enet_host_destroy(local->host);
        local->host = NULL;
    }
    time(&local->connect_start);
}

void mainmenu_cancel_connection(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    if (local->host) {
        enet_host_destroy(local->host);
        local->host = NULL;
    }
    local->connect_ip_input.disabled = 0;
    local->connect_ip_button.disabled = 0;
    menu_select(&local->connect_menu, &local->connect_ip_button);
    mainmenu_prev_menu(c, userdata);
}

void mainmenu_listen_for_connections(component *c, void *userdata) {
    scene *s = (scene*)userdata;
    mainmenu_local *local = scene_get_userdata(s);
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = settings_get()->net.net_listen_port;
    local->host = enet_host_create(&address, 1, 2, 0, 0);
    s->gs->role = ROLE_SERVER;
    if (local->host == NULL) {
        DEBUG("Failed to initialize ENet server");
        return;
    }
    enet_socket_set_option(local->host->socket, ENET_SOCKOPT_REUSEADDR, 1);
    mainmenu_enter_menu_listen(c, userdata);
}
*/

void mainmenu_free(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    menu_free(&local->main_menu);
    free(local);
    settings_save();
}

void mainmenu_tick(scene *scene, int paused) {
    mainmenu_local *local = scene_get_userdata(scene);
    //game_state *gs = scene->gs;

    // Tick menu
    menu_tick(&local->main_menu);

/*
    // Handle video confirm menu
    if(local->mstack[local->mstack_pos-1] == &local->video_confirm_menu) {
        if(difftime(time(NULL), local->video_accept_timer) >= 1.0) {
            time(&local->video_accept_timer);
            local->video_accept_secs--;
            if(sprintf(local->video_accept_label,
                       "ACCEPT NEW RESOLUTION? %d",
                       local->video_accept_secs) > 0) {
                ((textbutton*)local->video_confirm_header.obj)->text = local->video_accept_label;
            }
        }
        if(local->video_accept_secs == 0) {
            local->video_confirm_cancel.click(
                &local->video_confirm_cancel,
                local->video_confirm_cancel.userdata);
        }
    }

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


    // Handle network
    if (local->host) {
        ENetEvent event;
        if (enet_host_service(local->host, &event, 0) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            ENetPacket * packet = enet_packet_create("0", 2,  ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(local->host);
            if (gs->role == ROLE_SERVER) {
                DEBUG("client connected!");
                controller *player1_ctrl, *player2_ctrl;
                keyboard_keys *keys;
                game_player *p1 = game_state_get_player(gs, 0);
                game_player *p2 = game_state_get_player(gs, 1);

                // force the speed to 3
                game_state_set_speed(scene->gs, 5);

                p1->har_id = HAR_JAGUAR;
                p1->pilot_id = 0;
                p2->har_id = HAR_JAGUAR;
                p2->pilot_id = 0;

                player1_ctrl = malloc(sizeof(controller));
                controller_init(player1_ctrl);
                player1_ctrl->har = p1->har;
                player2_ctrl = malloc(sizeof(controller));
                controller_init(player2_ctrl);
                player2_ctrl->har = p2->har;

                // Player 1 controller -- Keyboard
                settings_keyboard *k = &settings_get()->keys;
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_GetScancodeFromName(k->key1_up);
                keys->down = SDL_GetScancodeFromName(k->key1_down);
                keys->left = SDL_GetScancodeFromName(k->key1_left);
                keys->right = SDL_GetScancodeFromName(k->key1_right);
                keys->punch = SDL_GetScancodeFromName(k->key1_punch);
                keys->kick = SDL_GetScancodeFromName(k->key1_kick);
                keys->escape = SDL_GetScancodeFromName(k->key1_escape);
                keyboard_create(player1_ctrl, keys, 0);
                game_player_set_ctrl(p1, player1_ctrl);

                // Player 2 controller -- Network
                net_controller_create(player2_ctrl, local->host, event.peer, ROLE_SERVER);
                game_player_set_ctrl(p2, player2_ctrl);
                local->host = NULL;
                game_player_set_selectable(p2, 1);

                chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)), AI_DIFFICULTY_CHAMPION);
                chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)), AI_DIFFICULTY_CHAMPION);

                game_state_set_next(gs, SCENE_MELEE);
            } else if (gs->role == ROLE_CLIENT) {
                DEBUG("connected to server!");
                controller *player1_ctrl, *player2_ctrl;
                keyboard_keys *keys;
                game_player *p1 = game_state_get_player(gs, 0);
                game_player *p2 = game_state_get_player(gs, 1);

                // force the speed to 3
                game_state_set_speed(scene->gs, 5);

                p1->har_id = HAR_JAGUAR;
                p1->pilot_id = 0;
                p2->har_id = HAR_JAGUAR;
                p2->pilot_id = 0;

                player1_ctrl = malloc(sizeof(controller));
                controller_init(player1_ctrl);
                player1_ctrl->har = p1->har;
                player2_ctrl = malloc(sizeof(controller));
                controller_init(player2_ctrl);
                player2_ctrl->har = p2->har;

                // Player 1 controller -- Network
                net_controller_create(player1_ctrl, local->host, event.peer, ROLE_CLIENT);
                game_player_set_ctrl(p1, player1_ctrl);

                // Player 2 controller -- Keyboard
                settings_keyboard *k = &settings_get()->keys;
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_GetScancodeFromName(k->key1_up);
                keys->down = SDL_GetScancodeFromName(k->key1_down);
                keys->left = SDL_GetScancodeFromName(k->key1_left);
                keys->right = SDL_GetScancodeFromName(k->key1_right);
                keys->punch = SDL_GetScancodeFromName(k->key1_punch);
                keys->kick = SDL_GetScancodeFromName(k->key1_kick);
                keys->escape = SDL_GetScancodeFromName(k->key1_escape);
                keyboard_create(player2_ctrl, keys, 0);
                game_player_set_ctrl(p2, player2_ctrl);
                local->host = NULL;
                game_player_set_selectable(p2, 1);

                chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)), AI_DIFFICULTY_CHAMPION);
                chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)), AI_DIFFICULTY_CHAMPION);

                game_state_set_next(gs, SCENE_MELEE);
            }
        } else {
            if (gs->role == ROLE_CLIENT && difftime(time(NULL), local->connect_start) > 5.0) {
                DEBUG("connection timed out");

                mainmenu_cancel_connection(&local->connect_ip_cancel_button, scene);
            }
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
                menu_handle_action(&local->main_menu, i->event_data.action);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);

/*
    mainmenu_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);

    i = p1;
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                // Skip repeated keys
                if(local->prev_key == i->event_data.action) {
                    continue;
                }
                local->prev_key = i->event_data.action;

                // Okay, the action is new. Handle it.
                if(i->event_data.action == ACT_ESC) {
                    if(local->current_menu == &local->main_menu) {
                        if(menu_selected(&local->main_menu) == &local->quit_button) {
                            game_state_set_next(scene->gs, SCENE_CREDITS);
                        } else {
                            menu_select(&local->main_menu, &local->quit_button);
                        }
                    } else {
                        if(local->host) {
                            enet_host_destroy(local->host);
                            local->host = NULL;
                        }
                        local->mstack[--local->mstack_pos] = NULL;
                        local->current_menu = local->mstack[local->mstack_pos-1];
                    }
                    sound_play(20, 0.5f, 0.0f, 2.0f);
                } else {
                    menu_handle_action(local->current_menu, i->event_data.action);
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
*/
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    mainmenu_local *local = scene_get_userdata(scene);
/*
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if (player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
            (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN
             && keyboard_binds_key(player1->ctrl, event))) {
        // these events will be handled by polling
        return 1;
    }
*/
    return menu_handle_event(&local->main_menu, event);
}

void mainmenu_render(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    menu_render(&local->main_menu);
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
    menu_create(&local->main_menu, 165, 5, 151, 119);
    menu_main_create(&local->main_menu);

    // Cleanups and resets
    for(int i = 0; i < 2; i++) {
        // destroy any leftover controllers
        controller *ctrl;
       if ((ctrl = game_player_get_ctrl(game_state_get_player(scene->gs, i)))) {
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

    // clear it, so this only happens the first time
    scene->gs->net_mode = NET_MODE_NONE;
*/

    // prev_key is used to prevent multiple clicks while key is down
    local->prev_key = ACT_PUNCH;

    // Music and renderer
    music_play(PSM_MENU);
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done
    return 0;
}
