#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "game/scene.h"
#include "game/scenes/mainmenu.h"
#include "game/menu/menu.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"

font font_large;
menu *current_menu;
menu main_menu;
component oneplayer_button;
component twoplayer_button;
component tourn_button;
component config_button;
component gameplay_button;
component ordering_button;
component help_button;
component demo_button;
component scoreboard_button;
component quit_button;

menu config_menu;
component playerone_input_button;
component playertwo_input_button;
component video_options_button;
component sound_toggle;
component music_toggle;
component stereo_toggle;
component config_done_button;

menu gameplay_menu;
component speed_slider;
component fightmode_toggle;
component powerone_slider;
component powertwo_slider;
component hazards_toggle;
component cpu_toggle;
component round_toggle;
component gameplay_done_button;


void mainmenu_quit(component *c, void *userdata) {
    scene *scene = userdata;
    scene->next_id = SCENE_CREDITS;
}

void mainmenu_1v1(component *c, void *userdata) {
    scene *scene = userdata;
    scene->next_id = SCENE_ARENA1;
}

void mainmenu_enter_menu(component *c, void *userdata) {
    current_menu = (menu*)userdata;
}

void mainmenu_prev_menu(component *c, void *userdata) {
    // TODO keep a stack of previous menus
    current_menu = &main_menu;
}

int mainmenu_init(scene *scene) {
    // Force music playback
    if(!music_playing()) {
        music_play("resources/MENU.PSM");
    }
    
    // Create font
    font_create(&font_large);
    if(font_load(&font_large, "resources/GRAPHCHR.DAT", FONT_BIG)) {
        PERROR("Error while loading large font!");
        font_free(&font_large);
        return 1;
    }
    
    // Create main menu
    menu_create(&main_menu, 165, 5, 151, 119);
    textbutton_create(&oneplayer_button, &font_large, "ONE PLAYER GAME");
    textbutton_create(&twoplayer_button, &font_large, "TWO PLAYER GAME");
    textbutton_create(&tourn_button, &font_large, "TOURNAMENT PLAY");
    textbutton_create(&config_button, &font_large, "CONFIGURATION");
    textbutton_create(&gameplay_button, &font_large, "GAMEPLAY");
    textbutton_create(&ordering_button, &font_large, "ORDERING INFO");
    textbutton_create(&help_button, &font_large, "HELP");
    textbutton_create(&demo_button, &font_large, "DEMO");
    textbutton_create(&scoreboard_button, &font_large, "SCOREBOARD");
    textbutton_create(&quit_button, &font_large, "QUIT");
    menu_attach(&main_menu, &oneplayer_button, 11);
    menu_attach(&main_menu, &twoplayer_button, 11);
    menu_attach(&main_menu, &tourn_button, 11);
    menu_attach(&main_menu, &config_button, 11);
    menu_attach(&main_menu, &gameplay_button, 11);
    menu_attach(&main_menu, &ordering_button, 11);
    menu_attach(&main_menu, &help_button, 11);
    menu_attach(&main_menu, &demo_button, 11);
    menu_attach(&main_menu, &scoreboard_button, 11);
    menu_attach(&main_menu, &quit_button, 11);

    // Status
    twoplayer_button.disabled = 1;
    tourn_button.disabled = 1;
    config_button.disabled = 0;
    gameplay_button.disabled = 0;
    ordering_button.disabled = 1;
    help_button.disabled = 1;
    demo_button.disabled = 1;
    scoreboard_button.disabled = 1;
    
    // Events
    quit_button.userdata = (void*)scene;
    quit_button.click = mainmenu_quit;
    oneplayer_button.userdata = (void*)scene;
    oneplayer_button.click = mainmenu_1v1;
    config_button.userdata = (void*)&config_menu;
    config_button.click = mainmenu_enter_menu;

    gameplay_button.userdata = (void*)&gameplay_menu;
    gameplay_button.click = mainmenu_enter_menu;

    // create configuration menu
    menu_create(&config_menu, 165, 5, 151, 119);
    textbutton_create(&playerone_input_button, &font_large, "PLAYER 1 INPUT");
    textbutton_create(&playertwo_input_button, &font_large, "PLAYER 2 INPUT");
    textbutton_create(&video_options_button, &font_large, "VIDEO OPTIONS");
    textselector_create(&sound_toggle, &font_large, "SOUND", "ON");
    textselector_add_option(&sound_toggle, "OFF");
    textselector_create(&music_toggle, &font_large, "MUSIC", "ON");
    textselector_add_option(&music_toggle, "OFF");
    textselector_create(&stereo_toggle, &font_large, "STEREO", "NORMAL");
    textselector_add_option(&stereo_toggle, "REVERSED");
    textbutton_create(&config_done_button, &font_large, "DONE");
    menu_attach(&config_menu, &playerone_input_button, 11);
    menu_attach(&config_menu, &playertwo_input_button, 11);
    menu_attach(&config_menu, &video_options_button, 11);
    menu_attach(&config_menu, &sound_toggle, 11);
    menu_attach(&config_menu, &music_toggle, 11);
    menu_attach(&config_menu, &stereo_toggle, 11);
    menu_attach(&config_menu, &config_done_button, 11);

    config_done_button.click = mainmenu_prev_menu;

    menu_create(&gameplay_menu, 165, 5, 151, 119);
    textslider_create(&speed_slider, &font_large, "SPEED", 10);
    textselector_create(&fightmode_toggle, &font_large, "FIGHT MODE", "NORMAL");
    textselector_add_option(&fightmode_toggle, "HYPER");
    textslider_create(&powerone_slider, &font_large, "POWER 1", 8);
    textslider_create(&powertwo_slider, &font_large, "POWER 2", 8);
    textselector_create(&hazards_toggle, &font_large, "HAZARDS", "ON");
    textselector_add_option(&hazards_toggle, "OFF");
    textselector_create(&cpu_toggle, &font_large, "CPU:", "PUNCHING BAG");
    textselector_add_option(&cpu_toggle, "ROOKIE");
    textselector_add_option(&cpu_toggle, "VETERAN");
    textselector_add_option(&cpu_toggle, "WORLD CLASS");
    textselector_add_option(&cpu_toggle, "CHAMPION");
    textselector_create(&round_toggle, &font_large, "", "ONE ROUND");
    textselector_add_option(&round_toggle, "BEST 2 OF 3");
    textselector_add_option(&round_toggle, "BEST 3 OF 5");
    textselector_add_option(&round_toggle, "BEST 4 OF 7");
    textbutton_create(&gameplay_done_button, &font_large, "DONE");
    menu_attach(&gameplay_menu, &speed_slider, 11);
    menu_attach(&gameplay_menu, &fightmode_toggle, 11);
    menu_attach(&gameplay_menu, &powerone_slider, 11);
    menu_attach(&gameplay_menu, &powertwo_slider, 11);
    menu_attach(&gameplay_menu, &hazards_toggle, 11);
    menu_attach(&gameplay_menu, &cpu_toggle, 11);
    menu_attach(&gameplay_menu, &round_toggle, 11);
    menu_attach(&gameplay_menu, &gameplay_done_button, 11);

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
    textbutton_free(&ordering_button);
    textbutton_free(&help_button);
    textbutton_free(&demo_button);
    textbutton_free(&scoreboard_button);
    textbutton_free(&quit_button);
    menu_free(&main_menu);

    textbutton_free(&playerone_input_button);
    textbutton_free(&playertwo_input_button);
    textbutton_free(&video_options_button);
    textselector_free(&sound_toggle);
    textselector_free(&music_toggle);
    textselector_free(&stereo_toggle);
    textbutton_free(&config_done_button);
    menu_free(&config_menu);

    textslider_free(&speed_slider);
    textselector_free(&fightmode_toggle);
    textslider_free(&powerone_slider);
    textslider_free(&powertwo_slider);
    textselector_free(&hazards_toggle);
    textselector_free(&cpu_toggle);
    textselector_free(&round_toggle);
    textbutton_free(&gameplay_done_button);
    menu_free(&gameplay_menu);

    font_free(&font_large);
}

void mainmenu_tick(scene *scene) {
    menu_tick(current_menu);
}

int mainmenu_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        if (current_menu == &main_menu) {
            scene->next_id = SCENE_CREDITS;
            return 1;
        } else {
            current_menu = &main_menu;
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

