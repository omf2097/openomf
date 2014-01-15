#include <SDL2/SDL.h>
#include <enet/enet.h>
#include <time.h>
#include <stdio.h>
#include "engine.h"
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scenes/mainmenu.h"
#include "game/menu/menu.h"
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

struct resolution_t {
    int w;  int h;  const char *name;
} _resolutions[] = {
    {320,   200,    "320x200"},
    {640,   400,    "640x400"},
    {800,   600,    "800x600"},
    {1024,  768,    "1024x768"},
    {1280,  720,    "1280x720"},
    {1280,  800,    "1280x800"},
    {1280,  1024,   "1280x1024"},
    {1440,  900,    "1440x900"},
    {1600,  1000,   "1600x1000"},
    {1600,  1200,   "1600x1200"},
    {1650,  1080,   "1650x1080"},
    {1920,  1080,   "1920x1080"},
    {1920,  1200,   "1920x1200"},
    {2560,  1440,   "2560x1440"},
    {2560,  1600,   "2560x1600"}
};

typedef struct resolution_t resolution;

typedef struct mainmenu_local_t {
    time_t connect_start;

    time_t video_accept_timer;
    settings_video old_video_settings;
    int video_accept_secs;
    char video_accept_label[100];

    char input_key_labels[6][100];
    int input_presskey_ready_ticks;
    int input_selected_player;

    char custom_resolution_label[40];
    vec2i custom_resolution;
    int is_custom_resolution;

    menu *current_menu;
    menu main_menu;
    component oneplayer_button;
    component twoplayer_button;
    component tourn_button;
    component config_button;
    component gameplay_button;
    component net_button;
    component help_button;
    component demo_button;
    component scoreboard_button;
    component quit_button;

    menu video_confirm_menu;
    component video_confirm_header;
    component video_confirm_cancel;
    component video_confirm_ok;

    menu net_menu;
    component net_header;
    component net_connect_button;
    component net_listen_button;
    component net_done_button;

    menu connect_menu;
    component connect_ip_input;
    component connect_ip_button;
    component connect_ip_cancel_button;

    menu listen_menu;
    component listen_button;
    component listen_cancel_button;

    menu video_menu;
    component video_header;
    component resolution_toggle;
    component vsync_toggle;
    component fullscreen_toggle;
    component scaling_toggle;
    component video_done_button;

    menu config_menu;
    component config_header;
    component playerone_input_button;
    component playertwo_input_button;
    component video_options_button;
    component sound_slider;
    component music_slider;
    component stereo_toggle;
    component config_done_button;

    menu input_config_menu;
    component input_config_header;
    component input_up_button;
    component input_down_button;
    component input_left_button;
    component input_right_button;
    component input_kick_button;
    component input_punch_button;
    component input_config_done_button;

    menu input_presskey_menu;
    component input_presskey_header;

    menu gameplay_menu;
    component gameplay_header;
    component speed_slider;
    component fightmode_toggle;
    component powerone_slider;
    component powertwo_slider;
    component hazards_toggle;
    component cpu_toggle;
    component round_toggle;
    component gameplay_done_button;

    ENetHost *host;

    // Menu stack
    menu *mstack[10];
    int mstack_pos;
} mainmenu_local;

void _setup_keyboard(game_state *gs, int players) {
    settings_keyboard *k = &settings_get()->keys;
    for(int i = 0; i < 2; i++) {
        // Set up controllers
        controller *ctrl = malloc(sizeof(controller));
        game_player *player = game_state_get_player(gs, i);
        controller_init(ctrl);

        // Set up keyboards
        keyboard_keys *keys = malloc(sizeof(keyboard_keys));
        if(i == 0) {
            keys->up = SDL_GetScancodeFromName(k->key1_up);
            keys->down = SDL_GetScancodeFromName(k->key1_down);
            keys->left = SDL_GetScancodeFromName(k->key1_left);
            keys->right = SDL_GetScancodeFromName(k->key1_right);
            keys->punch = SDL_GetScancodeFromName(k->key1_punch);
            keys->kick = SDL_GetScancodeFromName(k->key1_kick);
        } else {
            keys->up = SDL_GetScancodeFromName(k->key2_up);
            keys->down = SDL_GetScancodeFromName(k->key2_down);
            keys->left = SDL_GetScancodeFromName(k->key2_left);
            keys->right = SDL_GetScancodeFromName(k->key2_right);
            keys->punch = SDL_GetScancodeFromName(k->key2_punch);
            keys->kick = SDL_GetScancodeFromName(k->key2_kick);
        }
        keyboard_create(ctrl, keys, 0);

        // Set up player controller
        game_player_set_ctrl(player, ctrl);

        // If player is 2, set selectable to 0
        if(i == 1) {
            if(players == 1) { game_player_set_selectable(player, 0); }
            else if(players == 2) { game_player_set_selectable(player, 1); }
        }
    }
}

void _setup_joystick(game_state *gs, int players) {
    controller *ctrl = malloc(sizeof(controller));
    game_player *player = game_state_get_player(gs, 0);
    controller_init(ctrl);

    joystick_create(ctrl, 0);
    game_player_set_ctrl(player, ctrl);
    game_player_set_selectable(player, 0);


    ctrl = malloc(sizeof(controller));
    player = game_state_get_player(gs, 1);
    controller_init(ctrl);

    // Set up keyboards
    settings_keyboard *k = &settings_get()->keys;
    keyboard_keys *keys = malloc(sizeof(keyboard_keys));

    keys->up = SDL_GetScancodeFromName(k->key2_up);
    keys->down = SDL_GetScancodeFromName(k->key2_down);
    keys->left = SDL_GetScancodeFromName(k->key2_left);
    keys->right = SDL_GetScancodeFromName(k->key2_right);
    keys->punch = SDL_GetScancodeFromName(k->key2_punch);
    keys->kick = SDL_GetScancodeFromName(k->key2_kick);

    keyboard_create(ctrl, keys, 0);

    // Set up player controller
    game_player_set_ctrl(player, ctrl);

}



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

resolution *find_resolution_by_settings(settings *s) {
    int w = s->video.screen_w;
    int h = s->video.screen_h;

    for(int i = 0;i < sizeof(_resolutions)/sizeof(resolution);i++) {
        if(w == _resolutions[i].w && h == _resolutions[i].h) {
            return &_resolutions[i];
        }
    }
    return NULL;
}

// Menu event handlers
void mainmenu_quit(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_CREDITS);
}

void mainmenu_1v1(component *c, void *userdata) {
    scene *s = userdata;

    // Set up controllers
    _setup_keyboard(s->gs, 1);

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_1v2(component *c, void *userdata) {
    scene *s = userdata;

    // Set up controllers
    _setup_keyboard(s->gs, 2);

    // Load MELEE scene
    game_state_set_next(s->gs, SCENE_MELEE);
}

void mainmenu_tourn(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_MECHLAB);
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

void mainmenu_prev_menu(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[--local->mstack_pos] = NULL;
    local->current_menu = local->mstack[local->mstack_pos - 1];
}

void inputmenu_set_key(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    //settings_keyboard *k = &settings_get()->keys;
    local->mstack[local->mstack_pos++] = &local->input_presskey_menu;
    local->current_menu = &local->input_presskey_menu;
    local->input_presskey_ready_ticks = 0;
}

void video_done_clicked(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata); 
    settings_video *v = &settings_get()->video;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync);
    mainmenu_prev_menu(c, userdata);
    
    if(local->old_video_settings.screen_w != v->screen_w || 
        local->old_video_settings.screen_h != v->screen_h ||
        local->old_video_settings.fullscreen != v->fullscreen) {
        // Resolution confirmation dialog
        mainmenu_enter_menu_video_confirm(c, userdata);
        time(&local->video_accept_timer);
        local->video_accept_secs = 20;
        if(sprintf(local->video_accept_label, 
                   "ACCEPT NEW RESOLUTION? %d", 
                   local->video_accept_secs) > 0) {
            ((textbutton*)local->video_confirm_header.obj)->text = local->video_accept_label;
        }
    }
}

void video_confirm_cancel_clicked(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata); 
    settings_video *v = &settings_get()->video;
    *v = local->old_video_settings;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync);
    mainmenu_prev_menu(c, userdata);
}

void resolution_toggled(component *c, void *userdata, int pos) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    settings_video *v = &settings_get()->video;
    if(local->is_custom_resolution) {
        // The first index is always the custom resolution
        if(pos == 0) {
            v->screen_w = local->custom_resolution.x;
            v->screen_h = local->custom_resolution.y;
        } else {
            v->screen_w = _resolutions[pos-1].w;
            v->screen_h = _resolutions[pos-1].h;
        }
    } else {
        v->screen_w = _resolutions[pos].w;
        v->screen_h = _resolutions[pos].h;
    }
}

void menu_music_slide(component *c, void *userdata, int pos) {
    music_set_volume(pos/10.0f);
}

void menu_sound_slide(component *c, void *userdata, int pos) {
    sound_set_volume(pos/10.0f);
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
    free(settings_get()->net.net_server_ip);
    settings_get()->net.net_server_ip = dupestr(addr);
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
    address.port = settings_get()->net.net_server_port;

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
    address.port = settings_get()->net.net_server_port;
    local->host = enet_host_create(&address, 1, 2, 0, 0);
    s->gs->role = ROLE_SERVER;
    if (local->host == NULL) {
        DEBUG("Failed to initialize ENet server");
        return;
    }
    enet_socket_set_option(local->host->socket, ENET_SOCKOPT_REUSEADDR, 1);
    mainmenu_enter_menu_listen(c, userdata);
}


void mainmenu_free(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);

    textbutton_free(&local->oneplayer_button);
    textbutton_free(&local->twoplayer_button);
    textbutton_free(&local->tourn_button);
    textbutton_free(&local->config_button);
    textbutton_free(&local->gameplay_button);
    textbutton_free(&local->net_button);
    textbutton_free(&local->help_button);
    textbutton_free(&local->demo_button);
    textbutton_free(&local->scoreboard_button);
    textbutton_free(&local->quit_button);
    menu_free(&local->main_menu);

    textbutton_free(&local->config_header);
    textbutton_free(&local->playerone_input_button);
    textbutton_free(&local->playertwo_input_button);
    textbutton_free(&local->video_options_button);
    textslider_free(&local->sound_slider);
    textslider_free(&local->music_slider);
    textselector_free(&local->stereo_toggle);
    textbutton_free(&local->config_done_button);
    menu_free(&local->config_menu);

    textbutton_free(&local->video_header);
    textselector_free(&local->resolution_toggle);
    textselector_free(&local->vsync_toggle);
    textselector_free(&local->fullscreen_toggle);
    textselector_free(&local->scaling_toggle);
    textbutton_free(&local->video_done_button);
    menu_free(&local->video_menu);

    textbutton_free(&local->gameplay_header);
    textslider_free(&local->speed_slider);
    textselector_free(&local->fightmode_toggle);
    textslider_free(&local->powerone_slider);
    textslider_free(&local->powertwo_slider);
    textselector_free(&local->hazards_toggle);
    textselector_free(&local->cpu_toggle);
    textselector_free(&local->round_toggle);
    textbutton_free(&local->gameplay_done_button);
    menu_free(&local->gameplay_menu);

    textbutton_free(&local->input_config_header);
    textbutton_free(&local->input_up_button);
    textbutton_free(&local->input_down_button);
    textbutton_free(&local->input_left_button);
    textbutton_free(&local->input_right_button);
    textbutton_free(&local->input_punch_button);
    textbutton_free(&local->input_kick_button);
    textbutton_free(&local->input_config_done_button);
    menu_free(&local->input_config_menu);

    textbutton_free(&local->input_presskey_header);
    menu_free(&local->input_presskey_menu);

    textbutton_free(&local->net_header);
    textbutton_free(&local->net_connect_button);
    textbutton_free(&local->net_listen_button);
    textbutton_free(&local->net_done_button);
    menu_free(&local->net_menu);

    textinput_free(&local->connect_ip_input);
    textbutton_free(&local->connect_ip_button);
    textbutton_free(&local->connect_ip_cancel_button);
    menu_free(&local->connect_menu);

    textbutton_free(&local->listen_button);
    textbutton_free(&local->listen_cancel_button);
    menu_free(&local->listen_menu);

    settings_save();

    free(local);
}

void mainmenu_tick(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    game_state *gs = scene->gs;

    // Tick menu
    menu_tick(local->current_menu);

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
                    component *selected = *(component**)vector_get(&local->input_config_menu.objs, local->input_config_menu.selected);
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
                        }
                        update_keys(local, 2);
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
                game_state_set_speed(scene->gs, 3);

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
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_SCANCODE_UP;
                keys->down = SDL_SCANCODE_DOWN;
                keys->left = SDL_SCANCODE_LEFT;
                keys->right = SDL_SCANCODE_RIGHT;
                keys->punch = SDL_SCANCODE_RETURN;
                keys->kick = SDL_SCANCODE_RSHIFT;
                keyboard_create(player1_ctrl, keys, 0);
                game_player_set_ctrl(p1, player1_ctrl);

                // Player 2 controller -- Network
                net_controller_create(player2_ctrl, local->host, event.peer, ROLE_SERVER);
                game_player_set_ctrl(p2, player2_ctrl);
                local->host = NULL;
                game_player_set_selectable(p2, 1);
                game_state_set_next(gs, SCENE_MELEE);
            } else if (gs->role == ROLE_CLIENT) {
                DEBUG("connected to server!");
                controller *player1_ctrl, *player2_ctrl;
                keyboard_keys *keys;
                game_player *p1 = game_state_get_player(gs, 0);
                game_player *p2 = game_state_get_player(gs, 1);

                // force the speed to 3
                game_state_set_speed(scene->gs, 3);

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
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_SCANCODE_UP;
                keys->down = SDL_SCANCODE_DOWN;
                keys->left = SDL_SCANCODE_LEFT;
                keys->right = SDL_SCANCODE_RIGHT;
                keys->punch = SDL_SCANCODE_RETURN;
                keys->kick = SDL_SCANCODE_RSHIFT;
                keyboard_create(player2_ctrl, keys, 0);
                game_player_set_ctrl(p2, player2_ctrl);
                local->host = NULL;
                game_player_set_selectable(p2, 1);
                game_state_set_next(gs, SCENE_MELEE);
            }
        } else {
            if (gs->role == ROLE_CLIENT && difftime(time(NULL), local->connect_start) > 5.0) {
                DEBUG("connection timed out");

                mainmenu_cancel_connection(&local->connect_ip_cancel_button, scene);
            }
        }
    }
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    mainmenu_local *local = scene_get_userdata(scene);

    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        if(local->current_menu == &local->main_menu) {
            if(menu_selected(&local->main_menu) == &local->quit_button) {
                game_state_set_next(scene->gs, SCENE_CREDITS);
            } else {
                menu_select(&local->main_menu, &local->quit_button);
            }
            return 1;
        } else {
            if(local->host) {
                enet_host_destroy(local->host);
                local->host = NULL;
            }
            local->mstack[--local->mstack_pos] = NULL;
            local->current_menu = local->mstack[local->mstack_pos-1];
        }
    }
    return menu_handle_event(local->current_menu, event);
}

void mainmenu_render(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);
    menu_render(local->current_menu);
}

// Init menus
int mainmenu_create(scene *scene) {
    // Init local data
    mainmenu_local *local = malloc(sizeof(mainmenu_local));
    scene_set_userdata(scene, local);

    // Load settings
    settings *setting = settings_get();
    
    // Force music playback
    if(!music_playing()) {
        char filename[64];
        get_filename_by_id(PSM_MENU, filename);
        music_play(filename);
        music_set_volume(settings_get()->sound.music_vol/10.0f);
    }

    game_state_set_speed(scene->gs, settings_get()->gameplay.speed);

    // Zero out host
    local->host = NULL;
    
    // Start stack & set main menu to current
    local->mstack_pos = 0;
    local->mstack[local->mstack_pos++] = &local->main_menu;
    local->current_menu = &local->main_menu;

    // Create main menu
    menu_create(&local->main_menu, 165, 5, 151, 119);
    textbutton_create(&local->oneplayer_button, &font_large, "ONE PLAYER GAME");
    textbutton_create(&local->twoplayer_button, &font_large, "TWO PLAYER GAME");
    textbutton_create(&local->tourn_button, &font_large, "TOURNAMENT PLAY");
    textbutton_create(&local->net_button, &font_large, "NETWORK PLAY");
    textbutton_create(&local->config_button, &font_large, "CONFIGURATION");
    textbutton_create(&local->gameplay_button, &font_large, "GAMEPLAY");
    textbutton_create(&local->help_button, &font_large, "HELP");
    textbutton_create(&local->demo_button, &font_large, "DEMO");
    textbutton_create(&local->scoreboard_button, &font_large, "SCOREBOARD");
    textbutton_create(&local->quit_button, &font_large, "QUIT");
    menu_attach(&local->main_menu, &local->oneplayer_button, 11);
    menu_attach(&local->main_menu, &local->twoplayer_button, 11);
    menu_attach(&local->main_menu, &local->tourn_button, 11);
    menu_attach(&local->main_menu, &local->net_button, 11);
    menu_attach(&local->main_menu, &local->config_button, 11);
    menu_attach(&local->main_menu, &local->gameplay_button, 11);
    menu_attach(&local->main_menu, &local->help_button, 11);
    menu_attach(&local->main_menu, &local->demo_button, 11);
    menu_attach(&local->main_menu, &local->scoreboard_button, 11);
    menu_attach(&local->main_menu, &local->quit_button, 11);

    // Status
    local->tourn_button.disabled = 1;
    local->config_button.disabled = 0;
    local->gameplay_button.disabled = 0;
    local->net_button.disabled = 0;
    local->help_button.disabled = 1;
    local->demo_button.disabled = 1;
    local->scoreboard_button.disabled = 1;

    // Events
    local->quit_button.userdata = (void*)scene;
    local->quit_button.click = mainmenu_quit;
    local->oneplayer_button.userdata = (void*)scene;
    local->oneplayer_button.click = mainmenu_1v1;
    local->twoplayer_button.userdata = (void*)scene;
    local->twoplayer_button.click = mainmenu_1v2;
    local->tourn_button.userdata = (void*)scene;
    local->tourn_button.click = mainmenu_tourn;
    local->config_button.userdata = (void*)scene;
    local->config_button.click = mainmenu_enter_menu_config;
    local->net_button.userdata = (void*)scene;
    local->net_button.click = mainmenu_enter_menu_net;
    local->gameplay_button.userdata = (void*)scene;
    local->gameplay_button.click = mainmenu_enter_menu_gameplay;

    // destroy any leftover controllers
    controller *ctrl;
    for (int i = 0; i < 2; i++) {
       if ((ctrl = game_player_get_ctrl(game_state_get_player(scene->gs, i)))) {
           DEBUG("freeing controller");
           game_player_set_ctrl(game_state_get_player(scene->gs, i), NULL);
       }
    }

    // network play menu
    menu_create(&local->net_menu, 165, 5, 151, 119);
    textbutton_create(&local->net_header, &font_large, "NETWORK PLAY");
    textbutton_create(&local->net_connect_button, &font_large, "CONNECT TO SERVER");
    textbutton_create(&local->net_listen_button, &font_large, "START SERVER");
    textbutton_create(&local->net_done_button, &font_large, "DONE");
    menu_attach(&local->net_menu, &local->net_header, 33);
    menu_attach(&local->net_menu, &local->net_connect_button, 11);
    menu_attach(&local->net_menu, &local->net_listen_button, 55);
    menu_attach(&local->net_menu, &local->net_done_button, 11);

    local->net_listen_button.userdata = scene;
    local->net_listen_button.click = mainmenu_listen_for_connections;

    local->net_header.disabled = 1;
    menu_select(&local->net_menu, &local->net_connect_button);

    local->net_connect_button.userdata = scene;
    local->net_connect_button.click = mainmenu_enter_menu_connect;

    local->net_done_button.userdata = scene;
    local->net_done_button.click = mainmenu_prev_menu;

    // connect menu
    menu_create(&local->connect_menu, 10, 80, 300, 50);
    textinput_create(&local->connect_ip_input, &font_large, "Host/IP", setting->net.net_server_ip);
    textbutton_create(&local->connect_ip_button, &font_large, "CONNECT");
    textbutton_create(&local->connect_ip_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->connect_menu, &local->connect_ip_input, 11);
    menu_attach(&local->connect_menu, &local->connect_ip_button, 11);
    menu_attach(&local->connect_menu, &local->connect_ip_cancel_button, 11);

    local->connect_ip_button.userdata = scene;
    local->connect_ip_button.click = mainmenu_connect_to_ip;

    local->connect_ip_cancel_button.userdata = scene;
    local->connect_ip_cancel_button.click = mainmenu_cancel_connection;

    // listen menu
    menu_create(&local->listen_menu, 10, 80, 300, 50);
    textbutton_create(&local->listen_button, &font_large, "Waiting for connection...");
    textbutton_create(&local->listen_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->listen_menu, &local->listen_button, 11);
    menu_attach(&local->listen_menu, &local->listen_cancel_button, 11);
    local->listen_button.disabled = 1;
    menu_select(&local->listen_menu, &local->listen_cancel_button);

    local->listen_cancel_button.userdata = scene;
    local->listen_cancel_button.click = mainmenu_cancel_connection;

    // create configuration menu
    menu_create(&local->config_menu, 165, 5, 151, 119);
    textbutton_create(&local->config_header, &font_large, "CONFIGURATION");
    textbutton_create(&local->playerone_input_button, &font_large, "PLAYER 1 INPUT");
    textbutton_create(&local->playertwo_input_button, &font_large, "PLAYER 2 INPUT");
    textbutton_create(&local->video_options_button, &font_large, "VIDEO OPTIONS");
    textslider_create(&local->sound_slider, &font_large, "SOUND", 10, 1);
    textslider_create(&local->music_slider, &font_large, "MUSIC", 10, 1);
    textselector_create(&local->stereo_toggle, &font_large, "STEREO", "NORMAL");
    textselector_add_option(&local->stereo_toggle, "REVERSED");
    textbutton_create(&local->config_done_button, &font_large, "DONE");
    menu_attach(&local->config_menu, &local->config_header, 33);
    menu_attach(&local->config_menu, &local->playerone_input_button, 11);
    menu_attach(&local->config_menu, &local->playertwo_input_button, 11);
    menu_attach(&local->config_menu, &local->video_options_button, 11);
    menu_attach(&local->config_menu, &local->sound_slider, 11);
    menu_attach(&local->config_menu, &local->music_slider, 11);
    menu_attach(&local->config_menu, &local->stereo_toggle, 11);
    menu_attach(&local->config_menu, &local->config_done_button, 11);

    local->playerone_input_button.userdata = (void*)scene;
    local->playerone_input_button.click = mainmenu_enter_playerone_input_config;

    local->playertwo_input_button.userdata = (void*)scene;
    local->playertwo_input_button.click = mainmenu_enter_playertwo_input_config;

    local->video_options_button.userdata = (void*)scene;
    local->video_options_button.click = mainmenu_enter_menu_video;

    local->config_header.disabled = 1;
    menu_select(&local->config_menu, &local->playerone_input_button);

    local->config_done_button.click = mainmenu_prev_menu;
    local->config_done_button.userdata = (void*)scene;

    menu_create(&local->video_menu, 165, 5, 151, 119);
    textbutton_create(&local->video_header, &font_large, "VIDEO");
    resolution *res = find_resolution_by_settings(setting);
    if(res) {
        textselector_create(&local->resolution_toggle, &font_large, "RES:", _resolutions[0].name);
        local->is_custom_resolution = 0;
    } else {
        sprintf(local->custom_resolution_label, "%ux%u", setting->video.screen_w, setting->video.screen_h);
        textselector_create(&local->resolution_toggle, &font_large, "RES:", local->custom_resolution_label);
        local->custom_resolution.x = setting->video.screen_w;
        local->custom_resolution.y = setting->video.screen_h;
        local->is_custom_resolution = 1;
    }
    for(int i = local->is_custom_resolution ? 0 : 1;i < sizeof(_resolutions)/sizeof(resolution); ++i) {
        textselector_add_option(&local->resolution_toggle, _resolutions[i].name);
    }
    if(!local->is_custom_resolution) {
        textselector *t = local->resolution_toggle.obj;
        for(int i = 0;i < vector_size(&t->options);i++) {
            if(res->name == *(const char**)vector_get(&t->options, i)) {
                textselector_set_pos(&local->resolution_toggle, i);
                break;
            }
        }
    }

    textselector_create(&local->vsync_toggle, &font_large, "VSYNC:", "OFF");
    textselector_add_option(&local->vsync_toggle, "ON");
    textselector_create(&local->fullscreen_toggle, &font_large, "FULLSCREEN:", "OFF");
    textselector_add_option(&local->fullscreen_toggle, "ON");
    textselector_create(&local->scaling_toggle, &font_large, "SCALING:", "STRETCH");
    textselector_add_option(&local->scaling_toggle, "ASPECT");
    textselector_add_option(&local->scaling_toggle, "HQX");
    textbutton_create(&local->video_done_button, &font_large, "DONE");
    menu_attach(&local->video_menu, &local->video_header, 22);
    menu_attach(&local->video_menu, &local->resolution_toggle, 11);
    menu_attach(&local->video_menu, &local->vsync_toggle, 11);
    menu_attach(&local->video_menu, &local->fullscreen_toggle, 11);
    menu_attach(&local->video_menu, &local->scaling_toggle, 11);
    menu_attach(&local->video_menu, &local->video_done_button, 11);
    local->video_header.disabled = 1;
    local->video_done_button.click = video_done_clicked;
    local->video_done_button.userdata = (void*)scene;
    menu_select(&local->video_menu, &local->resolution_toggle);
    
    // Video confirmation dialog
    local->video_accept_secs = 0;
    menu_create(&local->video_confirm_menu, 10, 80, 300, 40);
    textbutton_create(&local->video_confirm_header, &font_large, "ACCEPT NEW RESOLUTION?");
    textbutton_create(&local->video_confirm_cancel, &font_large, "CANCEL");
    textbutton_create(&local->video_confirm_ok, &font_large, "OK");
    menu_attach(&local->video_confirm_menu, &local->video_confirm_header, 11);
    menu_attach(&local->video_confirm_menu, &local->video_confirm_cancel, 11);
    menu_attach(&local->video_confirm_menu, &local->video_confirm_ok, 11);
    
    local->video_confirm_header.disabled = 1;
    menu_select(&local->video_confirm_menu, &local->video_confirm_cancel);
    local->video_confirm_cancel.click = video_confirm_cancel_clicked;
    local->video_confirm_cancel.userdata = (void*)scene;
    local->video_confirm_ok.click = mainmenu_prev_menu;
    local->video_confirm_ok.userdata = (void*)scene;

    menu_create(&local->gameplay_menu, 165, 5, 151, 119);
    textbutton_create(&local->gameplay_header, &font_large, "GAMEPLAY");
    textslider_create(&local->speed_slider, &font_large, "SPEED", 10, 0);
    textselector_create(&local->fightmode_toggle, &font_large, "FIGHT MODE", "NORMAL");
    textselector_add_option(&local->fightmode_toggle, "HYPER");
    textslider_create(&local->powerone_slider, &font_large, "POWER 1", 8, 0);
    textslider_create(&local->powertwo_slider, &font_large, "POWER 2", 8, 0);
    textselector_create(&local->hazards_toggle, &font_large, "HAZARDS", "OFF");
    textselector_add_option(&local->hazards_toggle, "ON");
    textselector_create(&local->cpu_toggle, &font_large, "CPU:", "PUNCHING BAG");
    textselector_add_option(&local->cpu_toggle, "ROOKIE");
    textselector_add_option(&local->cpu_toggle, "VETERAN");
    textselector_add_option(&local->cpu_toggle, "WORLD CLASS");
    textselector_add_option(&local->cpu_toggle, "CHAMPION");
    textselector_add_option(&local->cpu_toggle, "DEADLY");
    textselector_add_option(&local->cpu_toggle, "ULTIMATE");
    textselector_create(&local->round_toggle, &font_large, "", "ONE ROUND");
    textselector_add_option(&local->round_toggle, "BEST 2 OF 3");
    textselector_add_option(&local->round_toggle, "BEST 3 OF 5");
    textselector_add_option(&local->round_toggle, "BEST 4 OF 7");
    textbutton_create(&local->gameplay_done_button, &font_large, "DONE");
    menu_attach(&local->gameplay_menu, &local->gameplay_header, 22);
    menu_attach(&local->gameplay_menu, &local->speed_slider, 11);
    menu_attach(&local->gameplay_menu, &local->fightmode_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->powerone_slider, 11);
    menu_attach(&local->gameplay_menu, &local->powertwo_slider, 11);
    menu_attach(&local->gameplay_menu, &local->hazards_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->cpu_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->round_toggle, 11);
    menu_attach(&local->gameplay_menu, &local->gameplay_done_button, 11);
    
    // sound options
    local->sound_slider.slide = menu_sound_slide;
    local->music_slider.slide = menu_music_slide;
    textslider_bindvar(&local->sound_slider, &setting->sound.sound_vol);
    textslider_bindvar(&local->music_slider, &setting->sound.music_vol);
    textselector_bindvar(&local->stereo_toggle, &setting->sound.stereo_reversed);
    
    // video options
    local->resolution_toggle.toggle = resolution_toggled;
    local->resolution_toggle.userdata = (void*)scene;
    textselector_bindvar(&local->vsync_toggle, &setting->video.vsync);
    textselector_bindvar(&local->fullscreen_toggle, &setting->video.fullscreen);
    textselector_bindvar(&local->scaling_toggle, &setting->video.scaling);

    // input config menu
    menu_create(&local->input_config_menu, 165, 5, 151, 119);
    textbutton_create(&local->input_config_header, &font_large, "CUSTOM INPUT SETUP");
    textbutton_create(&local->input_up_button, &font_large, "UP:");
    textbutton_create(&local->input_down_button, &font_large, "DOWN:");
    textbutton_create(&local->input_left_button, &font_large, "LEFT:");
    textbutton_create(&local->input_right_button, &font_large, "RIGHT:");
    textbutton_create(&local->input_punch_button, &font_large, "PUNCH:");
    textbutton_create(&local->input_kick_button, &font_large, "KICK:");
    textbutton_create(&local->input_config_done_button, &font_large, "DONE");
    menu_attach(&local->input_config_menu, &local->input_config_header, 22);
    menu_attach(&local->input_config_menu, &local->input_up_button, 11);
    menu_attach(&local->input_config_menu, &local->input_down_button, 11);
    menu_attach(&local->input_config_menu, &local->input_left_button, 11);
    menu_attach(&local->input_config_menu, &local->input_right_button, 11);
    menu_attach(&local->input_config_menu, &local->input_punch_button, 11);
    menu_attach(&local->input_config_menu, &local->input_kick_button, 11);
    menu_attach(&local->input_config_menu, &local->input_config_done_button, 11);

    local->input_config_header.disabled = 1;
    menu_select(&local->input_config_menu, &local->input_up_button);

    local->input_up_button.click = inputmenu_set_key;
    local->input_up_button.userdata = (void*)scene;

    local->input_down_button.click = inputmenu_set_key;
    local->input_down_button.userdata = (void*)scene;

    local->input_left_button.click = inputmenu_set_key;
    local->input_left_button.userdata = (void*)scene;

    local->input_right_button.click = inputmenu_set_key;
    local->input_right_button.userdata = (void*)scene;

    local->input_punch_button.click = inputmenu_set_key;
    local->input_punch_button.userdata = (void*)scene;

    local->input_kick_button.click = inputmenu_set_key;
    local->input_kick_button.userdata = (void*)scene;

    local->input_config_done_button.click = mainmenu_prev_menu;
    local->input_config_done_button.userdata = (void*)scene;

    // input press key menu
    menu_create(&local->input_presskey_menu, 10, 80, 300, 20);
    textbutton_create(&local->input_presskey_header, &font_large, "PRESS A KEY FOR THIS ACTION...");
    menu_attach(&local->input_presskey_menu, &local->input_presskey_header, 11);

    // gameplay options
    textslider_bindvar(&local->speed_slider, &setting->gameplay.speed);
    textslider_bindvar(&local->powerone_slider, &setting->gameplay.power1);
    textslider_bindvar(&local->powertwo_slider, &setting->gameplay.power2);
    textselector_bindvar(&local->fightmode_toggle, &setting->gameplay.fight_mode);
    textselector_bindvar(&local->hazards_toggle, &setting->gameplay.hazards_on);
    textselector_bindvar(&local->cpu_toggle, &setting->gameplay.difficulty);
    textselector_bindvar(&local->round_toggle, &setting->gameplay.rounds);

    local->gameplay_header.disabled = 1;
    menu_select(&local->gameplay_menu, &local->speed_slider);

    local->speed_slider.userdata = (void*)scene;
    local->speed_slider.slide = menu_speed_slide;

    local->gameplay_done_button.click = mainmenu_prev_menu;
    local->gameplay_done_button.userdata = (void*)scene;

    // Allocate memory for the input key labels
    ((textbutton*)local->input_up_button.obj)->text = local->input_key_labels[0];
    ((textbutton*)local->input_down_button.obj)->text = local->input_key_labels[1];
    ((textbutton*)local->input_left_button.obj)->text = local->input_key_labels[2];
    ((textbutton*)local->input_right_button.obj)->text = local->input_key_labels[3];
    ((textbutton*)local->input_punch_button.obj)->text = local->input_key_labels[4];
    ((textbutton*)local->input_kick_button.obj)->text = local->input_key_labels[5];

    // Set callbacks
    scene_set_event_cb(scene, mainmenu_event);
    //scene_set_render_cb(scene, mainmenu_render);
    scene_set_render_overlay_cb(scene, mainmenu_render);
    scene_set_free_cb(scene, mainmenu_free);
    scene_set_tick_cb(scene, mainmenu_tick);

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

    // All done
    return 0;
}
