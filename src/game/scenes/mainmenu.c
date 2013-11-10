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

enum {
    ROLE_SERVER,
    ROLE_CLIENT
};

typedef struct mainmenu_local_t {
    time_t connect_start;

    time_t video_accept_timer;
    settings_video old_video_settings;
    int video_accept_secs;
    char video_accept_label[100];

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
    component sound_toggle;
    component music_toggle;
    component stereo_toggle;
    component config_done_button;

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
    int role;

    // Menu stack
    menu *mstack[10];
    int mstack_pos;
} mainmenu_local;

// Menu event handlers
void mainmenu_quit(component *c, void *userdata) {
    game_state_set_next(SCENE_CREDITS);
}

void mainmenu_1v1(component *c, void *userdata) {
    // TODO: Read keys from settings later

    // Set up controllers
    for(int i = 0; i < 2; i++) {
        // Set up controllers
        controller *ctrl = malloc(sizeof(controller));
        game_player *player = game_state_get_player(i);
        controller_init(ctrl);

        // Set up keyboards
        keyboard_keys *keys = malloc(sizeof(keyboard_keys));
        if(i == 0) {
            keys->up = SDL_SCANCODE_UP;
            keys->down = SDL_SCANCODE_DOWN;
            keys->left = SDL_SCANCODE_LEFT;
            keys->right = SDL_SCANCODE_RIGHT;
            keys->punch = SDL_SCANCODE_RETURN;
            keys->kick = SDL_SCANCODE_RSHIFT;
        } else {
            keys->up = SDL_SCANCODE_W;
            keys->down = SDL_SCANCODE_S;
            keys->left = SDL_SCANCODE_A;
            keys->right = SDL_SCANCODE_D;
            keys->punch = SDL_SCANCODE_LSHIFT;
            keys->kick = SDL_SCANCODE_LCTRL;
        }
        keyboard_create(ctrl, keys);

        // Set up player controller
        game_player_set_ctrl(player, ctrl);

        // If player is 2, set selectable to 0
        if(i == 1) {
            game_player_set_selectable(player, 0);
        }
    }

    // Load MELEE scene
    game_state_set_next(SCENE_MELEE);
}

void mainmenu_1v2(component *c, void *userdata) {
    // TODO: Read keys from settings later

    // Set up controllers
    for(int i = 0; i < 2; i++) {
        // Set up controllers
        controller *ctrl = malloc(sizeof(controller));
        game_player *player = game_state_get_player(i);
        controller_init(ctrl);

        // Set up keyboards
        keyboard_keys *keys = malloc(sizeof(keyboard_keys));
        if(i == 0) {
            keys->up = SDL_SCANCODE_UP;
            keys->down = SDL_SCANCODE_DOWN;
            keys->left = SDL_SCANCODE_LEFT;
            keys->right = SDL_SCANCODE_RIGHT;
            keys->punch = SDL_SCANCODE_RETURN;
            keys->kick = SDL_SCANCODE_RSHIFT;
        } else {
            keys->up = SDL_SCANCODE_W;
            keys->down = SDL_SCANCODE_S;
            keys->left = SDL_SCANCODE_A;
            keys->right = SDL_SCANCODE_D;
            keys->punch = SDL_SCANCODE_LSHIFT;
            keys->kick = SDL_SCANCODE_LCTRL;
        }
        keyboard_create(ctrl, keys);

        // Set up player controller
        game_player_set_ctrl(player, ctrl);

        // If player is 2, set selectable to 0
        if(i == 1) {
            game_player_set_selectable(player, 1);
        }
    }

    // Load MELEE scene
    game_state_set_next(SCENE_MELEE);
}

void mainmenu_tourn(component *c, void *userdata) {
    game_state_set_next(SCENE_MECHLAB);
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

void mainmenu_prev_menu(component *c, void *userdata) {
    mainmenu_local *local = scene_get_userdata((scene*)userdata);
    local->mstack[--local->mstack_pos] = NULL;
    local->current_menu = local->mstack[local->mstack_pos - 1];
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
    settings_video *v = &settings_get()->video;
    v->screen_w = _resolutions[pos].w;
    v->screen_h = _resolutions[pos].h;
}

/*
void mainmenu_connect_to_ip(component *c, void *userdata) {
    ENetAddress address;
    char *addr = textinput_value(&connect_ip_input);
    host = enet_host_create(NULL, 1, 2, 0, 0);
    role = ROLE_CLIENT;
    if (host == NULL) {
        DEBUG("Failed to initialize ENet client");
        return;
    }
    connect_ip_input.disabled = 1;
    connect_ip_button.disabled = 1;
    menu_select(&connect_menu, &connect_ip_cancel_button);

    enet_address_set_host(&address, addr);
    address.port = 1337;

    ENetPeer *peer = enet_host_connect(host, &address, 2, 0);

    if (peer == NULL) {
        DEBUG("Unable to connect to %s", addr);
        enet_host_destroy(host);
        host = NULL;
    }
    time(&connect_start);
}

void mainmenu_cancel_connection(component *c, void *userdata) {
    if (host) {
        enet_host_destroy(host);
        host = NULL;
    }
    connect_ip_input.disabled = 0;
    connect_ip_button.disabled = 0;
    menu_select(&connect_menu, &connect_ip_button);
    mainmenu_prev_menu(c, userdata);
}

void mainmenu_listen_for_connections(component *c, void *userdata) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1337;
    host = enet_host_create(&address, 1, 2, 0, 0);
    role = ROLE_SERVER;
    if (host == NULL) {
        DEBUG("Failed to initialize ENet client");
        return;
    }
    mainmenu_enter_menu(c, userdata);
}
*/

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
    textselector_free(&local->sound_toggle);
    textselector_free(&local->music_toggle);
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

    /*textbutton_free(&local->net_header);
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
    menu_free(&local->listen_menu);*/

    settings_save();

    free(local);
}

void mainmenu_tick(scene *scene) {
    mainmenu_local *local = scene_get_userdata(scene);

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

/*
    // Handle network
    if (host) {
        ENetEvent event;
        if (enet_host_service(host, &event, 0) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            ENetPacket * packet = enet_packet_create("0", 2,  ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(host);
            if (role == ROLE_SERVER) {
                DEBUG("client connected!");
                controller *player1_ctrl, *player2_ctrl;
                keyboard_keys *keys;

                scene->player1.har_id = HAR_JAGUAR;
                scene->player1.player_id = 0;
                scene->player2.har_id = HAR_JAGUAR;
                scene->player2.player_id = 0;

                player1_ctrl = malloc(sizeof(controller));
                controller_init(player1_ctrl);
                player1_ctrl->har = scene->player1.har;
                player2_ctrl = malloc(sizeof(controller));
                controller_init(player2_ctrl);
                player2_ctrl->har = scene->player2.har;

                // Player 1 controller -- Keyboard
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_SCANCODE_UP;
                keys->down = SDL_SCANCODE_DOWN;
                keys->left = SDL_SCANCODE_LEFT;
                keys->right = SDL_SCANCODE_RIGHT;
                keys->punch = SDL_SCANCODE_RETURN;
                keys->kick = SDL_SCANCODE_RSHIFT;
                keyboard_create(player1_ctrl, keys);
                scene_set_player1_ctrl(scene, player1_ctrl);

                // Player 2 controller -- Network
                net_controller_create(player2_ctrl, host, event.peer);
                scene_set_player2_ctrl(scene, player2_ctrl);
                host = NULL;
                scene->player2.selectable = 1;
                scene->next_id = SCENE_MELEE;
            } else if (role == ROLE_CLIENT) {
                DEBUG("connected to server!");
                controller *player1_ctrl, *player2_ctrl;
                keyboard_keys *keys;

                scene->player1.har_id = HAR_JAGUAR;
                scene->player1.player_id = 0;
                scene->player2.har_id = HAR_JAGUAR;
                scene->player2.player_id = 0;

                player1_ctrl = malloc(sizeof(controller));
                controller_init(player1_ctrl);
                player1_ctrl->har = scene->player1.har;
                player2_ctrl = malloc(sizeof(controller));
                controller_init(player2_ctrl);
                player2_ctrl->har = scene->player2.har;

                // Player 1 controller -- Network
                net_controller_create(player1_ctrl, host, event.peer);
                scene_set_player1_ctrl(scene, player1_ctrl);

                // Player 2 controller -- Keyboard
                keys = malloc(sizeof(keyboard_keys));
                keys->up = SDL_SCANCODE_UP;
                keys->down = SDL_SCANCODE_DOWN;
                keys->left = SDL_SCANCODE_LEFT;
                keys->right = SDL_SCANCODE_RIGHT;
                keys->punch = SDL_SCANCODE_RETURN;
                keys->kick = SDL_SCANCODE_RSHIFT;
                keyboard_create(player2_ctrl, keys);
                scene_set_player2_ctrl(scene, player2_ctrl);
                host = NULL;
                scene->player2.selectable = 1;
                scene->next_id = SCENE_MELEE;
            }
        } else {
            if (role == ROLE_CLIENT && difftime(time(NULL), connect_start) > 5.0) {
                DEBUG("connection timed out");

                mainmenu_cancel_connection(&connect_ip_cancel_button, NULL);
            }
        }
    }*/
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    mainmenu_local *local = scene_get_userdata(scene);

    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        if(local->current_menu == &local->main_menu) {
            if(menu_selected(&local->main_menu) == &local->quit_button) {
                game_state_set_next(SCENE_CREDITS);
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
    local->net_button.disabled = 1;
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
    //local->net_button.userdata = (void*)&local->net_menu;
    //local->net_button.click = mainmenu_enter_menu;
    local->gameplay_button.userdata = (void*)scene;
    local->gameplay_button.click = mainmenu_enter_menu_gameplay;

/*
    // network play menu
    menu_create(&local->net_menu, 165, 5, 151, 119);
    textbutton_create(&local->net_header, &font_large, "NETWORK PLAY");
    textbutton_create(&local->net_connect_button, &font_large, "CONNECT TO SERVER");
    textbutton_create(&local->net_listen_button, &font_large, "START SERVER");
    textbutton_create(&local->net_done_button, &font_large, "DONE");
    menu_attach(&local->net_menu, &local->net_header, 33),
    menu_attach(&local->net_menu, &local->net_connect_button, 11),
    menu_attach(&local->net_menu, &local->net_listen_button, 55),
    menu_attach(&local->net_menu, &local->net_done_button, 11),

    local->net_listen_button.userdata = (void*)&local->listen_menu;
    local->net_listen_button.click = mainmenu_listen_for_connections;

    local->net_header.disabled = 1;
    menu_select(&local->net_menu, &local->net_connect_button);

    local->net_connect_button.userdata = (void*)&local->connect_menu;
    local->net_connect_button.click = mainmenu_enter_menu;

    local->net_done_button.click = mainmenu_prev_menu;

    // connect menu
    menu_create(&local->connect_menu, 10, 80, 300, 50);
    textinput_create(&local->connect_ip_input, &font_large, "Host/IP", "");
    textbutton_create(&local->connect_ip_button, &font_large, "CONNECT");
    textbutton_create(&local->connect_ip_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->connect_menu, &local->connect_ip_input, 11),
    menu_attach(&local->connect_menu, &local->connect_ip_button, 11),
    menu_attach(&local->connect_menu, &local->connect_ip_cancel_button, 11),

    local->connect_ip_button.userdata = (void*)&local->connect_menu;
    local->connect_ip_button.click = mainmenu_connect_to_ip;

    local->connect_ip_cancel_button.click = mainmenu_cancel_connection;

    // listen menu
    menu_create(&local->listen_menu, 10, 80, 300, 50);
    textbutton_create(&local->listen_button, &font_large, "Waiting for connection...");
    textbutton_create(&local->listen_cancel_button, &font_large, "CANCEL");
    menu_attach(&local->listen_menu, &local->listen_button, 11),
    menu_attach(&local->listen_menu, &local->listen_cancel_button, 11),
    local->listen_button.disabled = 1;
    menu_select(&local->listen_menu, &local->listen_cancel_button);

    local->listen_cancel_button.click = mainmenu_cancel_connection;
*/
    // create configuration menu
    menu_create(&local->config_menu, 165, 5, 151, 119);
    textbutton_create(&local->config_header, &font_large, "CONFIGURATION");
    textbutton_create(&local->playerone_input_button, &font_large, "PLAYER 1 INPUT");
    textbutton_create(&local->playertwo_input_button, &font_large, "PLAYER 2 INPUT");
    textbutton_create(&local->video_options_button, &font_large, "VIDEO OPTIONS");
    textselector_create(&local->sound_toggle, &font_large, "SOUND", "OFF");
    textselector_add_option(&local->sound_toggle, "ON");
    textselector_create(&local->music_toggle, &font_large, "MUSIC", "OFF");
    textselector_add_option(&local->music_toggle, "ON");
    textselector_create(&local->stereo_toggle, &font_large, "STEREO", "NORMAL");
    textselector_add_option(&local->stereo_toggle, "REVERSED");
    textbutton_create(&local->config_done_button, &font_large, "DONE");
    menu_attach(&local->config_menu, &local->config_header, 33);
    menu_attach(&local->config_menu, &local->playerone_input_button, 11);
    menu_attach(&local->config_menu, &local->playertwo_input_button, 11);
    menu_attach(&local->config_menu, &local->video_options_button, 11);
    menu_attach(&local->config_menu, &local->sound_toggle, 11);
    menu_attach(&local->config_menu, &local->music_toggle, 11);
    menu_attach(&local->config_menu, &local->stereo_toggle, 11);
    menu_attach(&local->config_menu, &local->config_done_button, 11);
    
    local->video_options_button.userdata = (void*)scene;
    local->video_options_button.click = mainmenu_enter_menu_video;

    local->config_header.disabled = 1;
    menu_select(&local->config_menu, &local->playerone_input_button);

    local->config_done_button.click = mainmenu_prev_menu;
    local->config_done_button.userdata = (void*)scene;

    menu_create(&local->video_menu, 165, 5, 151, 119);
    textbutton_create(&local->video_header, &font_large, "VIDEO");
    textselector_create(&local->resolution_toggle, &font_large, "RES:", _resolutions[0].name);
    for(int i=1;i < sizeof(_resolutions)/sizeof(struct resolution_t); ++i) {
        textselector_add_option(&local->resolution_toggle, _resolutions[i].name);
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
    textslider_create(&local->speed_slider, &font_large, "SPEED", 10);
    textselector_create(&local->fightmode_toggle, &font_large, "FIGHT MODE", "NORMAL");
    textselector_add_option(&local->fightmode_toggle, "HYPER");
    textslider_create(&local->powerone_slider, &font_large, "POWER 1", 8);
    textslider_create(&local->powertwo_slider, &font_large, "POWER 2", 8);
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
    textselector_bindvar(&local->sound_toggle, &setting->sound.sound_on);
    textselector_bindvar(&local->music_toggle, &setting->sound.music_on);
    textselector_bindvar(&local->stereo_toggle, &setting->sound.stereo_reversed);
    
    // video options
    local->resolution_toggle.toggle = resolution_toggled;
    textselector_bindvar(&local->resolution_toggle, &setting->video.resindex);
    textselector_bindvar(&local->vsync_toggle, &setting->video.vsync);
    textselector_bindvar(&local->fullscreen_toggle, &setting->video.fullscreen);
    textselector_bindvar(&local->scaling_toggle, &setting->video.scaling);
    
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

    local->gameplay_done_button.click = mainmenu_prev_menu;
    local->gameplay_done_button.userdata = (void*)scene;

    // Set callbacks
    scene_set_event_cb(scene, mainmenu_event);
    scene_set_render_cb(scene, mainmenu_render);
    scene_set_free_cb(scene, mainmenu_free);
    scene_set_tick_cb(scene, mainmenu_tick);

    // All done
    return 0;
}
