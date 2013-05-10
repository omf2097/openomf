#include "engine.h"
#include "game/scene.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/scenes/arena.h"
#include "game/scenes/progressbar.h"
#include "audio/music.h"
#include "game/settings.h"
#include "game/har.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/menu/menu.h"
#include "game/menu/menu_background.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "controller/controller.h"
#include "controller/keyboard.h"
#include "utils/log.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <shadowdive/shadowdive.h>

#define BAR_COLOR_BG color_create(89,40,101,255)
#define BAR_COLOR_TL_BORDER color_create(60,0,60,255)
#define BAR_COLOR_BR_BORDER color_create(178,0,223,255)
#define HEALTHBAR_COLOR_BG color_create(255,56,109,255)
#define HEALTHBAR_COLOR_TL_BORDER color_create(255,0,0,255)
#define HEALTHBAR_COLOR_BR_BORDER color_create(158,0,0,255)
#define ENDURANCEBAR_COLOR_BG color_create(97,150,186,255)
#define ENDURANCEBAR_COLOR_TL_BORDER color_create(24,117,138,255)
#define ENDURANCEBAR_COLOR_BR_BORDER color_create(0,69,93,255)

menu game_menu;
component title_button;
component return_button;
component sound_slider;
component music_slider;
component speed_slider;
component video_button;
component help_button;
component quit_button;
texture tex;
int menu_visible = 0;

progress_bar player1_health_bar;
progress_bar player2_health_bar;
progress_bar player1_endurance_bar;
progress_bar player2_endurance_bar;

void game_menu_quit(component *c, void *userdata) {
    scene *scene = userdata;
    scene_set_player1_har(scene, NULL);
    scene_set_player2_har(scene, NULL);
    scene_set_player1_ctrl(scene, NULL);
    scene_set_player2_ctrl(scene, NULL);
    scene->next_id = SCENE_MENU;
}

void game_menu_return(component *c, void *userdata) {
    menu_visible=0;
}

int arena_init(scene *scene) {
    settings *setting = settings_get();
    controller *player1_ctrl, *player2_ctrl;
    keyboard_keys *keys, *keys2;
    music_stop();
    switch (scene->bk->file_id) {
        case 8:
            music_play("resources/ARENA0.PSM");
            break;
        case 16:
            music_play("resources/ARENA1.PSM");
            break;
        case 32:
            music_play("resources/ARENA2.PSM");
            break;
        case 64:
            music_play("resources/ARENA3.PSM");
            break;
        case 128:
            music_play("resources/ARENA4.PSM");
            break;
    }

    player1_ctrl = malloc(sizeof(controller));
    keys = malloc(sizeof(keyboard_keys));
    keys->up = SDL_SCANCODE_UP;
    keys->down = SDL_SCANCODE_DOWN;
    keys->left = SDL_SCANCODE_LEFT;
    keys->right = SDL_SCANCODE_RIGHT;
    keys->punch = SDL_SCANCODE_RETURN;
    keys->kick = SDL_SCANCODE_RSHIFT;
    keyboard_create(player1_ctrl, scene->player1.har, keys);
    scene_set_player1_ctrl(scene, player1_ctrl);

    player2_ctrl = malloc(sizeof(controller));
    keys2 = malloc(sizeof(keyboard_keys));
    keys2->up = SDL_SCANCODE_W;
    keys2->down = SDL_SCANCODE_S;
    keys2->left = SDL_SCANCODE_A;
    keys2->right = SDL_SCANCODE_D;
    keys2->punch = SDL_SCANCODE_LSHIFT;
    keys2->kick = SDL_SCANCODE_LCTRL;
    keyboard_create(player2_ctrl, scene->player2.har, keys2);
    scene_set_player2_ctrl(scene, player2_ctrl);

    menu_create(&game_menu, 70, 5, 181, 117);
    textbutton_create(&title_button, &font_large, "OMF 2097");
    textbutton_create(&return_button, &font_large, "RETURN TO GAME");
    textslider_create(&sound_slider, &font_large, "SOUND", 10);
    textslider_create(&music_slider, &font_large, "MUSIC", 10);
    textslider_create(&speed_slider, &font_large, "SPEED", 10);
    textbutton_create(&video_button, &font_large, "VIDEO OPTIONS");
    textbutton_create(&help_button, &font_large, "HELP");
    textbutton_create(&quit_button, &font_large, "QUIT");

    menu_attach(&game_menu, &title_button, 33);
    menu_attach(&game_menu, &return_button, 11);
    menu_attach(&game_menu, &sound_slider, 11);
    menu_attach(&game_menu, &music_slider, 11);
    menu_attach(&game_menu, &speed_slider, 11);
    menu_attach(&game_menu, &video_button, 11);
    menu_attach(&game_menu, &help_button, 11);
    menu_attach(&game_menu, &quit_button, 11);
    
    // sound options
    textslider_bindvar(&sound_slider, &setting->sound.sound_vol);
    textslider_bindvar(&music_slider, &setting->sound.music_vol);
    
    // gameplay options
    textslider_bindvar(&speed_slider, &setting->gameplay.speed);

    title_button.disabled=1;

    // Events
    quit_button.userdata = (void*)scene;
    quit_button.click = game_menu_quit;
    return_button.click = game_menu_return;

    menu_select(&game_menu, &return_button);

    // background for the 'help' at the bottom of the screen
    // TODO support rendering text onto it
    menu_background_create(&tex, 301, 37);
    
    // Health bars
    progressbar_create(&player1_health_bar, 
                       5, 5, 100, 8, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       HEALTHBAR_COLOR_TL_BORDER,
                       HEALTHBAR_COLOR_BR_BORDER,
                       HEALTHBAR_COLOR_BG,
                       PROGRESSBAR_RIGHT);
    progressbar_create(&player2_health_bar, 
                       215, 5, 100, 8, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       HEALTHBAR_COLOR_TL_BORDER,
                       HEALTHBAR_COLOR_BR_BORDER,
                       HEALTHBAR_COLOR_BG,
                       PROGRESSBAR_LEFT);
    progressbar_create(&player1_endurance_bar, 
                       5, 14, 100, 4, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_RIGHT);
    progressbar_create(&player2_endurance_bar, 
                       215, 14, 100, 4, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_LEFT);
    return 0;
}

void arena_deinit(scene *scene) {
    menu_visible = 0;
    scene_set_player1_har(scene, NULL);
    scene_set_player2_har(scene, NULL);
    scene_set_player1_ctrl(scene, NULL);
    scene_set_player2_ctrl(scene, NULL);
    
    textbutton_free(&title_button);
    textbutton_free(&return_button);
    textslider_free(&sound_slider);
    textslider_free(&music_slider);
    textslider_free(&speed_slider);
    textbutton_free(&video_button);
    textbutton_free(&help_button);
    textbutton_free(&quit_button);
    menu_free(&game_menu);

    texture_free(&tex);

    music_stop();
    
    progressbar_free(&player1_health_bar);
    progressbar_free(&player2_health_bar);
    progressbar_free(&player1_endurance_bar);
    progressbar_free(&player2_endurance_bar);
    
    settings_save(settings_get());
}

void arena_tick(scene *scene) {
    if(!menu_visible) {
        keyboard_tick(scene->player1.ctrl);
        keyboard_tick(scene->player2.ctrl);
        
        // Collision detections
        har_collision_har(scene->player1.har, scene->player2.har);
        har_collision_har(scene->player2.har, scene->player1.har);
        har_collision_scene(scene->player1.har, scene);
        har_collision_scene(scene->player2.har, scene);
        
        // Turn the HARs to face the enemy
        if (scene->player1.har->phy.pos.x > scene->player2.har->phy.pos.x) {
            if (scene->player1.har->direction == 1) {
                har_set_direction(scene->player1.har, -1);
                har_set_direction(scene->player2.har, 1);
            }
        } else if (scene->player1.har->phy.pos.x < scene->player2.har->phy.pos.x) {
            if (scene->player1.har->direction == -1) {
                har_set_direction(scene->player1.har, 1);
                har_set_direction(scene->player2.har, -1);
            }
        }
    }
}

int arena_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            if (!menu_visible) {
                menu_visible = 1;
            } else {
                menu_visible = 0;
            }
            return 1;
        }
        break;
    }
    if(menu_visible) {
        return menu_handle_event(&game_menu, e);
    } else {
        // TODO don't assume a keyboard controller!
        keyboard_handle(scene->player1.ctrl, e);
        keyboard_handle(scene->player2.ctrl, e);
        return 0;
    }
}

void arena_render(scene *scene) {
    // Set health bar
    if(scene->player1.har != NULL && scene->player2.har != NULL) {
        float p1_hp = (float)scene->player1.har->health / (float)scene->player1.har->health_max;
        float p2_hp = (float)scene->player2.har->health / (float)scene->player2.har->health_max;
        progressbar_set(&player1_health_bar, p1_hp * 100);
        progressbar_set(&player2_health_bar, p2_hp * 100);
        progressbar_render(&player1_health_bar);
        progressbar_render(&player2_health_bar);

        // Set endurance bar
        float p1_en = (float)scene->player1.har->endurance / (float)scene->player1.har->endurance_max;
        float p2_en = (float)scene->player2.har->endurance / (float)scene->player2.har->endurance_max;
        progressbar_set(&player1_endurance_bar, p1_en * 100);
        progressbar_set(&player2_endurance_bar, p2_en * 100);
        progressbar_render(&player1_endurance_bar);
        progressbar_render(&player2_endurance_bar);

        // Render HAR and pilot names
        font_render(&font_small, lang_get(scene->player1.player_id+20), 5, 19, 186, 250, 250);
        font_render(&font_small, lang_get(scene->player1.har_id+31), 5, 26, 186, 250, 250);
        int h2len = strlen(lang_get(scene->player2.player_id+20)) * font_small.w;
        int p2len = strlen(lang_get(scene->player2.har_id+31)) * font_small.w;
        font_render(&font_small, lang_get(scene->player2.player_id+20), 315-p2len, 19, 186, 250, 250);
        font_render(&font_small, lang_get(scene->player2.har_id+31), 315-h2len, 26, 186, 250, 250);
    }

    // Draw menu if necessary
    if (menu_visible) {
        menu_render(&game_menu);
        video_render_sprite(&tex, 10, 150, BLEND_ALPHA_FULL);
    }
}

void arena_load(scene *scene) {
    scene->event = arena_event;
    scene->render = arena_render;
    scene->init = arena_init;
    scene->deinit = arena_deinit;
    scene->tick = arena_tick;
}
