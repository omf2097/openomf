#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include <enet/enet.h>
#include <time.h>
#include <stdio.h>
#include "engine.h"
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scene.h"
#include "game/scenes/mainmenu.h"
#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "game/menu/textinput.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "controller/net_controller.h"

struct resolution_t {
    int w;  int h;  const char *name;
} _resolutions[] = {
    {320,   200,    "320x200"},
    {640,   400,    "640x400"},
    {1280,  800,    "1280x800"},
    {1600,  1000,   "1600x1000"},
    {1650,  1080,   "1650x1080"},
    {1920,  1080,   "1920x1080"},
    {1920,  1200,   "1920x1200"},
    {2560,  1440,   "2560x1440"},
    {2560,  1600,   "2560x1600"}
};

time_t connect_start;

time_t video_accept_timer;
settings_video old_video_settings;
int video_accept_secs = 0;
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

enum {
    ROLE_SERVER,
    ROLE_CLIENT
};

// Menu stack
menu *mstack[10];
int mstack_pos = 0;

// Menu event handlers
void mainmenu_quit(component *c, void *userdata) {
    scene *scene = userdata;
    scene->next_id = SCENE_CREDITS;
}

void mainmenu_1v1(component *c, void *userdata) {
    scene *scene = userdata;

    controller *player1_ctrl, *player2_ctrl;
    keyboard_keys *keys, *keys2;

    //TODO read key config from settings

    // Player 1 controller
    player1_ctrl = malloc(sizeof(controller));
    controller_init(player1_ctrl);
    player1_ctrl->har = scene->player1.har;
    keys = malloc(sizeof(keyboard_keys));
    keys->up = SDL_SCANCODE_UP;
    keys->down = SDL_SCANCODE_DOWN;
    keys->left = SDL_SCANCODE_LEFT;
    keys->right = SDL_SCANCODE_RIGHT;
    keys->punch = SDL_SCANCODE_RETURN;
    keys->kick = SDL_SCANCODE_RSHIFT;
    keyboard_create(player1_ctrl, keys);
    scene_set_player1_ctrl(scene, player1_ctrl);

    // TODO this should be AI
    // Player 2 controller
    player2_ctrl = malloc(sizeof(controller));
    controller_init(player2_ctrl);
    player2_ctrl->har = scene->player2.har;
    keys2 = malloc(sizeof(keyboard_keys));
    keys2->up = SDL_SCANCODE_W;
    keys2->down = SDL_SCANCODE_S;
    keys2->left = SDL_SCANCODE_A;
    keys2->right = SDL_SCANCODE_D;
    keys2->punch = SDL_SCANCODE_LSHIFT;
    keys2->kick = SDL_SCANCODE_LCTRL;
    keyboard_create(player2_ctrl, keys2);
    scene_set_player2_ctrl(scene, player2_ctrl);

    scene->player2.selectable = 0;
    scene->next_id = SCENE_MELEE;
}

void mainmenu_1v2(component *c, void *userdata) {
    scene *scene = userdata;

    controller *player1_ctrl, *player2_ctrl;
    keyboard_keys *keys, *keys2;

    // Player 1 controller
    player1_ctrl = malloc(sizeof(controller));
    controller_init(player1_ctrl);
    player1_ctrl->har = scene->player1.har;
    keys = malloc(sizeof(keyboard_keys));
    keys->up = SDL_SCANCODE_UP;
    keys->down = SDL_SCANCODE_DOWN;
    keys->left = SDL_SCANCODE_LEFT;
    keys->right = SDL_SCANCODE_RIGHT;
    keys->punch = SDL_SCANCODE_RETURN;
    keys->kick = SDL_SCANCODE_RSHIFT;
    keyboard_create(player1_ctrl, keys);
    scene_set_player1_ctrl(scene, player1_ctrl);

    // Player 2 controller
    player2_ctrl = malloc(sizeof(controller));
    controller_init(player2_ctrl);
    player2_ctrl->har = scene->player2.har;
    keys2 = malloc(sizeof(keyboard_keys));
    keys2->up = SDL_SCANCODE_W;
    keys2->down = SDL_SCANCODE_S;
    keys2->left = SDL_SCANCODE_A;
    keys2->right = SDL_SCANCODE_D;
    keys2->punch = SDL_SCANCODE_LSHIFT;
    keys2->kick = SDL_SCANCODE_LCTRL;
    keyboard_create(player2_ctrl, keys2);
    scene_set_player2_ctrl(scene, player2_ctrl);


    scene->player2.selectable = 1;
    scene->next_id = SCENE_MELEE;
}

void mainmenu_tourn(component *c, void *userdata) {
    scene *scene = userdata;
    scene->next_id = SCENE_MECHLAB;
}

void mainmenu_enter_menu(component *c, void *userdata) {
    current_menu = (menu*)userdata;
    mstack[mstack_pos++] = current_menu;
}

void mainmenu_prev_menu(component *c, void *userdata) {
    DEBUG("prev menu");
    mstack[--mstack_pos] = NULL;
    current_menu = mstack[mstack_pos-1];
}

void video_options_enter(component *c, void *userdata) {
    mainmenu_enter_menu(c, userdata);
    old_video_settings = settings_get()->video;
}

void video_done_clicked(component *c, void *userdata) {    
    settings_video *v = &settings_get()->video;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync);
    mainmenu_prev_menu(c, userdata);
    
    if (old_video_settings.screen_w != v->screen_w || 
        old_video_settings.screen_h != v->screen_h ||
        old_video_settings.fullscreen != v->fullscreen) {
        // Resolution confirmation dialog
        mainmenu_enter_menu(c, &video_confirm_menu);
        time(&video_accept_timer);
        video_accept_secs = 20;
        if(sprintf(video_accept_label, "ACCEPT NEW RESOLUTION? %d", video_accept_secs) > 0) {
            ((textbutton*)video_confirm_header.obj)->text = video_accept_label;
        }
    }
}

void video_confirm_cancel_clicked(component *c, void *userdata) {
    settings_video *v = &settings_get()->video;
    *v = old_video_settings;
    video_reinit(v->screen_w, v->screen_h, v->fullscreen, v->vsync);
    mainmenu_prev_menu(c, userdata);
}

void resolution_toggled(component *c, void *userdata, int pos) {
    settings_get()->video.screen_w = _resolutions[pos].w;
    settings_get()->video.screen_h = _resolutions[pos].h;
}

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

// Init menus
int mainmenu_init(scene *scene) {
    settings *setting = settings_get();
    
    // Force music playback
    if(!music_playing()) {
        music_play("resources/MENU.PSM");
        audio_set_volume(TYPE_MUSIC, setting->sound.music_vol/10.0f);
    }
    
    // Start stack
    mstack_pos = 0;
    mstack[mstack_pos++] = &main_menu;
    
    // Create main menu
    menu_create(&main_menu, 165, 5, 151, 119);
    textbutton_create(&oneplayer_button, &font_large, "ONE PLAYER GAME");
    textbutton_create(&twoplayer_button, &font_large, "TWO PLAYER GAME");
    textbutton_create(&tourn_button, &font_large, "TOURNAMENT PLAY");
    textbutton_create(&net_button, &font_large, "NETWORK PLAY");
    textbutton_create(&config_button, &font_large, "CONFIGURATION");
    textbutton_create(&gameplay_button, &font_large, "GAMEPLAY");
    textbutton_create(&help_button, &font_large, "HELP");
    textbutton_create(&demo_button, &font_large, "DEMO");
    textbutton_create(&scoreboard_button, &font_large, "SCOREBOARD");
    textbutton_create(&quit_button, &font_large, "QUIT");
    menu_attach(&main_menu, &oneplayer_button, 11);
    menu_attach(&main_menu, &twoplayer_button, 11);
    menu_attach(&main_menu, &tourn_button, 11);
    menu_attach(&main_menu, &net_button, 11);
    menu_attach(&main_menu, &config_button, 11);
    menu_attach(&main_menu, &gameplay_button, 11);
    menu_attach(&main_menu, &help_button, 11);
    menu_attach(&main_menu, &demo_button, 11);
    menu_attach(&main_menu, &scoreboard_button, 11);
    menu_attach(&main_menu, &quit_button, 11);

    // Status
    tourn_button.disabled = 0;
    config_button.disabled = 0;
    gameplay_button.disabled = 0;
    net_button.disabled = 0;
    help_button.disabled = 1;
    demo_button.disabled = 1;
    scoreboard_button.disabled = 1;
    
    // Events
    quit_button.userdata = (void*)scene;
    quit_button.click = mainmenu_quit;
    oneplayer_button.userdata = (void*)scene;
    oneplayer_button.click = mainmenu_1v1;
    twoplayer_button.userdata = (void*)scene;
    twoplayer_button.click = mainmenu_1v2;
    tourn_button.userdata = (void*)scene;
    tourn_button.click = mainmenu_tourn;
    config_button.userdata = (void*)&config_menu;
    config_button.click = mainmenu_enter_menu;
    net_button.userdata = (void*)&net_menu;
    net_button.click = mainmenu_enter_menu;


    gameplay_button.userdata = (void*)&gameplay_menu;
    gameplay_button.click = mainmenu_enter_menu;
    
    // Video confirmation dialog
    menu_create(&video_confirm_menu, 10, 80, 300, 40);
    textbutton_create(&video_confirm_header, &font_large, "ACCEPT NEW RESOLUTION?");
    textbutton_create(&video_confirm_cancel, &font_large, "CANCEL");
    textbutton_create(&video_confirm_ok, &font_large, "OK");
    menu_attach(&video_confirm_menu, &video_confirm_header, 11);
    menu_attach(&video_confirm_menu, &video_confirm_cancel, 11);
    menu_attach(&video_confirm_menu, &video_confirm_ok, 11);
    
    video_confirm_header.disabled = 1;
    menu_select(&video_confirm_menu, &video_confirm_cancel);
    
    video_confirm_cancel.click = video_confirm_cancel_clicked;
    
    video_confirm_ok.click = mainmenu_prev_menu;
    
    
    // network play menu
    menu_create(&net_menu, 165, 5, 151, 119);
    textbutton_create(&net_header, &font_large, "NETWORK PLAY");
    textbutton_create(&net_connect_button, &font_large, "CONNECT TO SERVER");
    textbutton_create(&net_listen_button, &font_large, "START SERVER");
    textbutton_create(&net_done_button, &font_large, "DONE");
    menu_attach(&net_menu, &net_header, 33),
    menu_attach(&net_menu, &net_connect_button, 11),
    menu_attach(&net_menu, &net_listen_button, 55),
    menu_attach(&net_menu, &net_done_button, 11),

    net_listen_button.userdata = (void*)&listen_menu;
    net_listen_button.click = mainmenu_listen_for_connections;

    net_header.disabled = 1;
    menu_select(&net_menu, &net_connect_button);

    net_connect_button.userdata = (void*)&connect_menu;
    net_connect_button.click = mainmenu_enter_menu;

    net_done_button.click = mainmenu_prev_menu;

    // connect menu
    menu_create(&connect_menu, 10, 80, 300, 50);
    textinput_create(&connect_ip_input, &font_large, "Host/IP", "");
    textbutton_create(&connect_ip_button, &font_large, "CONNECT");
    textbutton_create(&connect_ip_cancel_button, &font_large, "CANCEL");
    menu_attach(&connect_menu, &connect_ip_input, 11),
    menu_attach(&connect_menu, &connect_ip_button, 11),
    menu_attach(&connect_menu, &connect_ip_cancel_button, 11),

    connect_ip_button.userdata = (void*)&connect_menu;
    connect_ip_button.click = mainmenu_connect_to_ip;

    connect_ip_cancel_button.click = mainmenu_cancel_connection;

    // listen menu
    menu_create(&listen_menu, 10, 80, 300, 50);
    textbutton_create(&listen_button, &font_large, "Waiting for connection...");
    textbutton_create(&listen_cancel_button, &font_large, "CANCEL");
    menu_attach(&listen_menu, &listen_button, 11),
    menu_attach(&listen_menu, &listen_cancel_button, 11),
    listen_button.disabled = 1;
    menu_select(&listen_menu, &listen_cancel_button);

    listen_cancel_button.click = mainmenu_cancel_connection;

    // create configuration menu
    menu_create(&config_menu, 165, 5, 151, 119);
    textbutton_create(&config_header, &font_large, "CONFIGURATION");
    textbutton_create(&playerone_input_button, &font_large, "PLAYER 1 INPUT");
    textbutton_create(&playertwo_input_button, &font_large, "PLAYER 2 INPUT");
    textbutton_create(&video_options_button, &font_large, "VIDEO OPTIONS");
    textselector_create(&sound_toggle, &font_large, "SOUND", "OFF");
    textselector_add_option(&sound_toggle, "ON");
    textselector_create(&music_toggle, &font_large, "MUSIC", "OFF");
    textselector_add_option(&music_toggle, "ON");
    textselector_create(&stereo_toggle, &font_large, "STEREO", "NORMAL");
    textselector_add_option(&stereo_toggle, "REVERSED");
    textbutton_create(&config_done_button, &font_large, "DONE");
    menu_attach(&config_menu, &config_header, 33);
    menu_attach(&config_menu, &playerone_input_button, 11);
    menu_attach(&config_menu, &playertwo_input_button, 11);
    menu_attach(&config_menu, &video_options_button, 11);
    menu_attach(&config_menu, &sound_toggle, 11);
    menu_attach(&config_menu, &music_toggle, 11);
    menu_attach(&config_menu, &stereo_toggle, 11);
    menu_attach(&config_menu, &config_done_button, 11);
    
    video_options_button.userdata = (void*)&video_menu;
    video_options_button.click = video_options_enter;

    config_header.disabled = 1;
    menu_select(&config_menu, &playerone_input_button);

    config_done_button.click = mainmenu_prev_menu;

    menu_create(&video_menu, 165, 5, 151, 119);
    textbutton_create(&video_header, &font_large, "VIDEO");
    textselector_create(&resolution_toggle, &font_large, "RES:", _resolutions[0].name);
    for(int i=1;i < sizeof(_resolutions)/sizeof(struct resolution_t); ++i) {
        textselector_add_option(&resolution_toggle, _resolutions[i].name);
    }
    textselector_create(&vsync_toggle, &font_large, "VSYNC:", "OFF");
    textselector_add_option(&vsync_toggle, "ON");
    textselector_create(&fullscreen_toggle, &font_large, "FULLSCREEN:", "OFF");
    textselector_add_option(&fullscreen_toggle, "ON");
    textselector_create(&scaling_toggle, &font_large, "SCALING:", "STRETCH");
    textselector_add_option(&scaling_toggle, "ASPECT");
    textselector_add_option(&scaling_toggle, "HQX");
    textbutton_create(&video_done_button, &font_large, "DONE");
    menu_attach(&video_menu, &video_header, 22);
    menu_attach(&video_menu, &resolution_toggle, 11);
    menu_attach(&video_menu, &vsync_toggle, 11);
    menu_attach(&video_menu, &fullscreen_toggle, 11);
    menu_attach(&video_menu, &scaling_toggle, 11);
    menu_attach(&video_menu, &video_done_button, 11);
    video_header.disabled = 1;
    video_done_button.click = video_done_clicked;
    menu_select(&video_menu, &resolution_toggle);
    
    menu_create(&gameplay_menu, 165, 5, 151, 119);
    textbutton_create(&gameplay_header, &font_large, "GAMEPLAY");
    textslider_create(&speed_slider, &font_large, "SPEED", 10);
    textselector_create(&fightmode_toggle, &font_large, "FIGHT MODE", "NORMAL");
    textselector_add_option(&fightmode_toggle, "HYPER");
    textslider_create(&powerone_slider, &font_large, "POWER 1", 8);
    textslider_create(&powertwo_slider, &font_large, "POWER 2", 8);
    textselector_create(&hazards_toggle, &font_large, "HAZARDS", "OFF");
    textselector_add_option(&hazards_toggle, "ON");
    textselector_create(&cpu_toggle, &font_large, "CPU:", "PUNCHING BAG");
    textselector_add_option(&cpu_toggle, "ROOKIE");
    textselector_add_option(&cpu_toggle, "VETERAN");
    textselector_add_option(&cpu_toggle, "WORLD CLASS");
    textselector_add_option(&cpu_toggle, "CHAMPION");
    textselector_add_option(&cpu_toggle, "DEADLY");
    textselector_add_option(&cpu_toggle, "ULTIMATE");
    textselector_create(&round_toggle, &font_large, "", "ONE ROUND");
    textselector_add_option(&round_toggle, "BEST 2 OF 3");
    textselector_add_option(&round_toggle, "BEST 3 OF 5");
    textselector_add_option(&round_toggle, "BEST 4 OF 7");
    textbutton_create(&gameplay_done_button, &font_large, "DONE");
    menu_attach(&gameplay_menu, &gameplay_header, 22);
    menu_attach(&gameplay_menu, &speed_slider, 11);
    menu_attach(&gameplay_menu, &fightmode_toggle, 11);
    menu_attach(&gameplay_menu, &powerone_slider, 11);
    menu_attach(&gameplay_menu, &powertwo_slider, 11);
    menu_attach(&gameplay_menu, &hazards_toggle, 11);
    menu_attach(&gameplay_menu, &cpu_toggle, 11);
    menu_attach(&gameplay_menu, &round_toggle, 11);
    menu_attach(&gameplay_menu, &gameplay_done_button, 11);
    
    // sound options
    textselector_bindvar(&sound_toggle, &setting->sound.sound_on);
    textselector_bindvar(&music_toggle, &setting->sound.music_on);
    textselector_bindvar(&stereo_toggle, &setting->sound.stereo_reversed);
    
    // video options
    resolution_toggle.toggle = resolution_toggled;
    textselector_bindvar(&resolution_toggle, &setting->video.resindex);
    textselector_bindvar(&vsync_toggle, &setting->video.vsync);
    textselector_bindvar(&fullscreen_toggle, &setting->video.fullscreen);
    textselector_bindvar(&scaling_toggle, &setting->video.scaling);
    
    // gameplay options
    textslider_bindvar(&speed_slider, &setting->gameplay.speed);
    textslider_bindvar(&powerone_slider, &setting->gameplay.power1);
    textslider_bindvar(&powertwo_slider, &setting->gameplay.power2);
    textselector_bindvar(&fightmode_toggle, &setting->gameplay.fight_mode);
    textselector_bindvar(&hazards_toggle, &setting->gameplay.hazards_on);
    textselector_bindvar(&cpu_toggle, &setting->gameplay.difficulty);
    textselector_bindvar(&round_toggle, &setting->gameplay.rounds);

    gameplay_header.disabled = 1;
    menu_select(&gameplay_menu, &speed_slider);

    gameplay_done_button.click = mainmenu_prev_menu;

    current_menu = &main_menu;
    
    // All done
    return 0;
}

void mainmenu_deinit(scene *scene) {      
    textbutton_free(&oneplayer_button);
    textbutton_free(&twoplayer_button);
    textbutton_free(&tourn_button);
    textbutton_free(&config_button);
    textbutton_free(&gameplay_button);
    textbutton_free(&net_button);
    textbutton_free(&help_button);
    textbutton_free(&demo_button);
    textbutton_free(&scoreboard_button);
    textbutton_free(&quit_button);
    menu_free(&main_menu);

    textbutton_free(&config_header);
    textbutton_free(&playerone_input_button);
    textbutton_free(&playertwo_input_button);
    textbutton_free(&video_options_button);
    textselector_free(&sound_toggle);
    textselector_free(&music_toggle);
    textselector_free(&stereo_toggle);
    textbutton_free(&config_done_button);
    menu_free(&config_menu);

    textbutton_free(&video_header);
    textselector_free(&resolution_toggle);
    textselector_free(&vsync_toggle);
    textselector_free(&fullscreen_toggle);
    textselector_free(&scaling_toggle);
    textbutton_free(&video_done_button);
    menu_free(&video_menu);

    textbutton_free(&gameplay_header);
    textslider_free(&speed_slider);
    textselector_free(&fightmode_toggle);
    textslider_free(&powerone_slider);
    textslider_free(&powertwo_slider);
    textselector_free(&hazards_toggle);
    textselector_free(&cpu_toggle);
    textselector_free(&round_toggle);
    textbutton_free(&gameplay_done_button);
    menu_free(&gameplay_menu);

    settings_save();
}

void mainmenu_tick(scene *scene) {
    menu_tick(current_menu);
    if (mstack[mstack_pos-1] == &video_confirm_menu) {
        if (difftime(time(NULL), video_accept_timer) >= 1.0) {
            time(&video_accept_timer);
            video_accept_secs--;
            if(sprintf(video_accept_label, "ACCEPT NEW RESOLUTION? %d", video_accept_secs) > 0) {
                ((textbutton*)video_confirm_header.obj)->text = video_accept_label;
            }
        }
        
        if (video_accept_secs == 0) {
            video_confirm_cancel.click(&video_confirm_cancel, video_confirm_cancel.userdata);
        }
    }

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
    }
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        DEBUG("esc key");
        if (current_menu == &main_menu) {
            if (menu_selected(&main_menu) == &quit_button) {
                scene->next_id = SCENE_CREDITS;
            } else {
                menu_select(&main_menu, &quit_button);
            }
            return 1;
        } else {
            if (host) {
                enet_host_destroy(host);
                host = NULL;
            }
            mstack[--mstack_pos] = NULL;
            current_menu = mstack[mstack_pos-1];
        }
    }
    return menu_handle_event(current_menu, event);
}

void mainmenu_render(scene *scene) {
    menu_render(current_menu);
}

void mainmenu_load(scene *scene) {
    scene->event = mainmenu_event;
    scene->render = mainmenu_render;
    scene->init = mainmenu_init;
    scene->deinit = mainmenu_deinit;
    scene->tick = mainmenu_tick;
}

