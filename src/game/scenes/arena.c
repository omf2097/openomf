#include "engine.h"
#include "game/scene.h"
#include "video/texture.h"
#include "video/video.h"
#include "game/scenes/arena.h"
#include "game/scenes/progressbar.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "game/settings.h"
#include "game/har.h"
#include "game/score.h"
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
#include <string.h>
#include <chipmunk/chipmunk.h>
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
#define TEXT_COLOR color_create(186,250,250,255)

typedef struct arena_local_t {
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
    int menu_visible;

    cpSpace *space;
    cpShape *line_floor;
    cpShape *line_wall_left;
    cpShape *line_wall_right;
    cpShape *line_ceiling;
    
    progress_bar player1_health_bar;
    progress_bar player2_health_bar;
    progress_bar player1_endurance_bar;
    progress_bar player2_endurance_bar;
    chr_score player1_score;
    chr_score player2_score;
} arena_local;

void game_menu_quit(component *c, void *userdata) {
    scene *scene = userdata;
    scene_set_player1_har(scene, NULL);
    scene_set_player2_har(scene, NULL);
    scene_set_player1_ctrl(scene, NULL);
    scene_set_player2_ctrl(scene, NULL);
    scene->next_id = SCENE_MENU;
}

void game_menu_return(component *c, void *userdata) {
    arena_local *local = ((scene*)userdata)->local;
    local->menu_visible = 0;
}

void music_slide(component *c, void *userdata, int pos) {
    audio_set_volume(TYPE_MUSIC, pos/10.0f);
}

void sound_slide(component *c, void *userdata, int pos) {
    audio_set_volume(TYPE_EFFECT, pos/10.0f);
}

int arena_init(scene *scene) {
    settings *setting;
    controller *player1_ctrl, *player2_ctrl;
    keyboard_keys *keys, *keys2;
    arena_local *local;
    cpVect grav;
    
    // Load up settings
    setting = settings_get();
    
    // Handle music playback
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
    audio_set_volume(TYPE_MUSIC, setting->sound.music_vol/10.0f);

    // Initialize local struct
    local = malloc(sizeof(arena_local));
    scene->local = local;
    
    // Init physics
    grav = cpv(0, 100);
    local->space = cpSpaceNew();
    cpSpaceSetGravity(local->space, grav);
    
    // Arena constraints
    local->line_floor = cpSegmentShapeNew(local->space->staticBody, cpv(0, 200), cpv(320, 200), 0);
    local->line_ceiling = cpSegmentShapeNew(local->space->staticBody, cpv(0, 0), cpv(320, 0), 0);
    local->line_wall_left = cpSegmentShapeNew(local->space->staticBody, cpv(0, 0), cpv(0, 200), 0);
    local->line_wall_right = cpSegmentShapeNew(local->space->staticBody, cpv(320, 0), cpv(320, 200), 0);
    cpSpaceAddShape(local->space, local->line_floor);
    cpSpaceAddShape(local->space, local->line_ceiling);
    cpSpaceAddShape(local->space, local->line_wall_left);
    cpSpaceAddShape(local->space, local->line_wall_right);
    
    // Init physics for hars
    har_init_physics(scene->player1.har, local->space);
    har_init_physics(scene->player2.har, local->space);
    
    // Player 1 controller
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

    // Player 2 controller
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

    // Arena menu
    local->menu_visible = 0;
    menu_create(&local->game_menu, 70, 5, 181, 117);
    textbutton_create(&local->title_button, &font_large, "OMF 2097");
    textbutton_create(&local->return_button, &font_large, "RETURN TO GAME");
    textslider_create(&local->sound_slider, &font_large, "SOUND", 10);
    textslider_create(&local->music_slider, &font_large, "MUSIC", 10);
    textslider_create(&local->speed_slider, &font_large, "SPEED", 10);
    textbutton_create(&local->video_button, &font_large, "VIDEO OPTIONS");
    textbutton_create(&local->help_button, &font_large, "HELP");
    textbutton_create(&local->quit_button, &font_large, "QUIT");

    menu_attach(&local->game_menu, &local->title_button, 33);
    menu_attach(&local->game_menu, &local->return_button, 11);
    menu_attach(&local->game_menu, &local->sound_slider, 11);
    menu_attach(&local->game_menu, &local->music_slider, 11);
    menu_attach(&local->game_menu, &local->speed_slider, 11);
    menu_attach(&local->game_menu, &local->video_button, 11);
    menu_attach(&local->game_menu, &local->help_button, 11);
    menu_attach(&local->game_menu, &local->quit_button, 11);
    
    // sound options
    local->sound_slider.slide = sound_slide;
    local->music_slider.slide = music_slide;
    textslider_bindvar(&local->sound_slider, &setting->sound.sound_vol);
    textslider_bindvar(&local->music_slider, &setting->sound.music_vol);
    
    // gameplay options
    textslider_bindvar(&local->speed_slider, &setting->gameplay.speed);

    local->title_button.disabled = 1;

    // Events
    local->quit_button.userdata = (void*)scene;
    local->quit_button.click = game_menu_quit;
    local->return_button.click = game_menu_return;

    menu_select(&local->game_menu, &local->return_button);

    // background for the 'help' at the bottom of the screen
    // TODO support rendering text onto it
    menu_background_create(&local->tex, 301, 37);
    
    // Health bars
    progressbar_create(&local->player1_health_bar, 
                       5, 5, 100, 8, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       HEALTHBAR_COLOR_TL_BORDER,
                       HEALTHBAR_COLOR_BR_BORDER,
                       HEALTHBAR_COLOR_BG,
                       PROGRESSBAR_RIGHT);
    progressbar_create(&local->player2_health_bar, 
                       215, 5, 100, 8, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       HEALTHBAR_COLOR_TL_BORDER,
                       HEALTHBAR_COLOR_BR_BORDER,
                       HEALTHBAR_COLOR_BG,
                       PROGRESSBAR_LEFT);
    progressbar_create(&local->player1_endurance_bar, 
                       5, 14, 100, 4, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_RIGHT);
    progressbar_create(&local->player2_endurance_bar, 
                       215, 14, 100, 4, 
                       BAR_COLOR_TL_BORDER, 
                       BAR_COLOR_BR_BORDER, 
                       BAR_COLOR_BG, 
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_LEFT);
    chr_score_create(&local->player1_score, 4, 33, 1.0f);
    chr_score_create(&local->player2_score, 215, 33, 1.0f); // TODO: Set better coordinates for this
    return 0;
}

void arena_deinit(scene *scene) {
    arena_local *local = scene->local;
    local->menu_visible = 0;
    scene_set_player1_har(scene, NULL);
    scene_set_player2_har(scene, NULL);
    scene_set_player1_ctrl(scene, NULL);
    scene_set_player2_ctrl(scene, NULL);
    
    textbutton_free(&local->title_button);
    textbutton_free(&local->return_button);
    textslider_free(&local->sound_slider);
    textslider_free(&local->music_slider);
    textslider_free(&local->speed_slider);
    textbutton_free(&local->video_button);
    textbutton_free(&local->help_button);
    textbutton_free(&local->quit_button);
    menu_free(&local->game_menu);

    texture_free(&local->tex);

    music_stop();
    
    progressbar_free(&local->player1_health_bar);
    progressbar_free(&local->player2_health_bar);
    progressbar_free(&local->player1_endurance_bar);
    progressbar_free(&local->player2_endurance_bar);
    chr_score_free(&local->player1_score);
    chr_score_free(&local->player2_score);
    
    settings_save();
    
    cpShapeFree(local->line_floor);
    cpShapeFree(local->line_ceiling);
    cpShapeFree(local->line_wall_left);
    cpShapeFree(local->line_wall_right);
    cpSpaceFree(local->space);
    
    free(scene->local);
}

void arena_tick(scene *scene) {
    arena_local *local = scene->local;

    // Tick physics
    cpSpaceStep(local->space, 0.08); // TODO: This is a guesstimate. Do we even need the real value here ?
    
    // Har ticks
    har_tick(scene->player1.har);
    har_tick(scene->player2.har);
    
    // Handle scrolling score texts
    chr_score_tick(&local->player1_score);
    chr_score_tick(&local->player2_score);

    // Handle menu, if visible
    if(!local->menu_visible) {
        keyboard_tick(scene->player1.ctrl);
        keyboard_tick(scene->player2.ctrl);
        
        // Collision detections
        har_collision_har(scene->player1.har, scene->player2.har);
        har_collision_har(scene->player2.har, scene->player1.har);
        // XXX this can't go in har.c because of a typedef loop on OSX
        //har_collision_scene(scene->player1.har, scene);
        //har_collision_scene(scene->player2.har, scene);
        
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
    arena_local *local = scene->local;

    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            if (!local->menu_visible) {
                local->menu_visible = 1;
            } else {
                local->menu_visible = 0;
            }
            return 1;
        }
        break;
    }
    if(local->menu_visible) {
        return menu_handle_event(&local->game_menu, e);
    } else {
        // TODO don't assume a keyboard controller!
        keyboard_handle(scene->player1.ctrl, e);
        keyboard_handle(scene->player2.ctrl, e);
        return 0;
    }
}

void arena_render(scene *scene) {
    arena_local *local = (arena_local*)scene->local;

    // Render hars
    har_render(scene->player1.har);
    har_render(scene->player2.har);

    // Debug textures
    if(scene->player1.har && scene->player1.har->cd_debug_tex.data) {
        video_render_sprite_flip(&scene->player1.har->cd_debug_tex, -50, -50, BLEND_ALPHA_FULL, FLIP_NONE);
    }
    if(scene->player2.har && scene->player2.har->cd_debug_tex.data) {
        video_render_sprite_flip(&scene->player2.har->cd_debug_tex, -50, -50, BLEND_ALPHA_FULL, FLIP_NONE);
    }
    
    // Set health bar
    if(scene->player1.har != NULL && scene->player2.har != NULL) {
        float p1_hp = (float)scene->player1.har->health / (float)scene->player1.har->health_max;
        float p2_hp = (float)scene->player2.har->health / (float)scene->player2.har->health_max;
        progressbar_set(&local->player1_health_bar, p1_hp * 100);
        progressbar_set(&local->player2_health_bar, p2_hp * 100);
        progressbar_render(&local->player1_health_bar);
        progressbar_render(&local->player2_health_bar);

        // Set endurance bar
        float p1_en = (float)scene->player1.har->endurance / (float)scene->player1.har->endurance_max;
        float p2_en = (float)scene->player2.har->endurance / (float)scene->player2.har->endurance_max;
        progressbar_set(&local->player1_endurance_bar, p1_en * 100);
        progressbar_set(&local->player2_endurance_bar, p2_en * 100);
        progressbar_render(&local->player1_endurance_bar);
        progressbar_render(&local->player2_endurance_bar);

        // Render HAR and pilot names
        font_render(&font_small, lang_get(scene->player1.player_id+20), 5, 19, TEXT_COLOR);
        font_render(&font_small, lang_get(scene->player1.har_id+31), 5, 26, TEXT_COLOR);
        int p2len = (strlen(lang_get(scene->player2.player_id+20))-1) * font_small.w;
        int h2len = (strlen(lang_get(scene->player2.har_id+31))-1) * font_small.w;
        font_render(&font_small, lang_get(scene->player2.player_id+20), 315-p2len, 19, TEXT_COLOR);
        font_render(&font_small, lang_get(scene->player2.har_id+31), 315-h2len, 26, TEXT_COLOR);
        
        // Render score stuff
        chr_score_render(&local->player1_score);
        chr_score_render(&local->player2_score);
        char tmp[50];
        chr_score_format(&local->player1_score, tmp);
        font_render(&font_small, tmp, 5, 33, TEXT_COLOR);
        int s2len = strlen(tmp) * font_small.w;
        chr_score_format(&local->player2_score, tmp);
        font_render(&font_small, tmp, 315-s2len, 33, TEXT_COLOR);
    }

    // Draw menu if necessary
    if(local->menu_visible) {
        menu_render(&local->game_menu);
        video_render_sprite(&local->tex, 10, 150, BLEND_ALPHA_FULL);
    }
}

void arena_load(scene *scene) {
    scene->event = arena_event;
    scene->render = arena_render;
    scene->init = arena_init;
    scene->deinit = arena_deinit;
    scene->tick = arena_tick;
}
