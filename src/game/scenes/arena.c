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
#include "controller/net_controller.h"
#include "utils/log.h"
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
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
    
    progress_bar player1_health_bar;
    progress_bar player2_health_bar;
    progress_bar player1_endurance_bar;
    progress_bar player2_endurance_bar;
    chr_score player1_score;
    chr_score player2_score;
} arena_local;

void game_menu_quit(component *c, void *userdata) {
    scene *scene = userdata;

    if(scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {
        net_controller_free(scene->player1.ctrl);
    }

    if(scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {
        net_controller_free(scene->player2.ctrl);
    }

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
    arena_local *local;
    /*cpVect grav;*/
    
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

    // init HARs
    har *h1, *h2;

    h1 = malloc(sizeof(har));
    h2 = malloc(sizeof(har));
    if (har_load(h1, scene->bk->palettes[0], scene->player1.har_id, 1)) {
        free(h1);
        free(h2);
        scene->next_id = SCENE_NONE;
        return 1;
    }
    if (har_load(h2, scene->bk->palettes[0], scene->player2.har_id, -1)) {
        har_free(h1);
        free(h2);
        scene->next_id = SCENE_NONE;
        return 1;
    }

    scene_set_player1_har(scene, h1);
    scene_set_player2_har(scene, h2);

    scene->player1.ctrl->har = h1;
    scene->player2.ctrl->har = h2;

    // remove the keyboard hooks
    // set up the magic HAR hooks
    if (scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(scene->player2.ctrl);
        har_add_hook(scene->player2.har, scene->player1.ctrl->har_hook, (void*)scene->player1.ctrl);
    }

    if (scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(scene->player1.ctrl);
        har_add_hook(scene->player1.har, scene->player2.ctrl->har_hook, (void*)scene->player2.ctrl);
    }

    // Arena constraints
    /*
    local->line_floor = cpSegmentShapeNew(global_space->staticBody, cpv(0, 200), cpv(320, 200), 0);
    local->line_ceiling = cpSegmentShapeNew(global_space->staticBody, cpv(0, 0), cpv(320, 0), 0);
    local->line_wall_left = cpSegmentShapeNew(global_space->staticBody, cpv(0, 0), cpv(0, 200), 0);
    local->line_wall_right = cpSegmentShapeNew(global_space->staticBody, cpv(320, 0), cpv(320, 200), 0);
    cpShapeSetFriction(local->line_floor, 1.0f);
    cpShapeSetElasticity(local->line_floor, 1.0f);
    cpSpaceAddShape(global_space, local->line_floor);
    cpSpaceAddShape(global_space, local->line_ceiling);
    cpSpaceAddShape(global_space, local->line_wall_left);
    cpSpaceAddShape(global_space, local->line_wall_right);
    */
    
    // Init physics for hars
    har_init(scene->player1.har, 60, 190);
    har_init(scene->player2.har, 260, 190);
    
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
    local->return_button.userdata = (void*)scene;
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
    
    /*
    cpSpaceRemoveShape(global_space, local->line_floor);
    cpSpaceRemoveShape(global_space, local->line_ceiling);
    cpSpaceRemoveShape(global_space, local->line_wall_left);
    cpSpaceRemoveShape(global_space, local->line_wall_right);
    cpShapeFree(local->line_floor);
    cpShapeFree(local->line_ceiling);
    cpShapeFree(local->line_wall_left);
    cpShapeFree(local->line_wall_right);
    */
    
    free(scene->local);
}

void arena_tick(scene *scene) {
    arena_local *local = scene->local;

    // Har ticks
    har_tick(scene->player1.har);
    har_tick(scene->player2.har);
    
    // Handle scrolling score texts
    chr_score_tick(&local->player1_score);
    chr_score_tick(&local->player2_score);

    // Handle menu, if visible
    if(!local->menu_visible) {
        ctrl_event *p1 = NULL, *p2 = NULL, *i;
        if(controller_tick(scene->player1.ctrl, &p1) ||
                controller_tick(scene->player2.ctrl, &p2)) {
            // one of the controllers bailed

            if(scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {
                net_controller_free(scene->player1.ctrl);
            }

            if(scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {
                net_controller_free(scene->player2.ctrl);
            }
            scene->next_id = SCENE_MENU;
        }

        i = p1;
        if (i) {
            do {
                /*DEBUG("har 1 act %d", i->action);*/
                har_act(scene->player1.har, i->action);
            } while((i = i->next));
            /*DEBUG("done");*/
        }
        i = p2;
        if (i) {
            do {
                /*DEBUG("har 2 act %d", i->action);*/
                har_act(scene->player2.har, i->action);
            } while((i = i->next));
            /*DEBUG("done");*/
        }
        
        // Collision detections
        har_collision_har(scene->player1.har, scene->player2.har);
        har_collision_har(scene->player2.har, scene->player1.har);
        // XXX this can't go in har.c because of a typedef loop on OSX
        //har_collision_scene(scene->player1.har, scene);
        //har_collision_scene(scene->player2.har, scene);
        
        // Turn the HARs to face the enemy
        int x1, x2, y1, y2;
        object_get_pos(&(scene->player1.har->pobj), &x1, &y1);
        object_get_pos(&(scene->player2.har->pobj), &x2, &y2);
        if (x1 > x2) {
            if (scene->player1.har->direction == 1) {
                har_set_direction(scene->player1.har, -1);
                har_set_direction(scene->player2.har, 1);
            }
        } else if (x1 < x2) {
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
        ctrl_event *p1=NULL, *p2=NULL, *i;
        controller_event(scene->player1.ctrl, e, &p1);
        controller_event(scene->player2.ctrl, e, &p2);
        i = p1;
        if (i) {
            do {
                /*DEBUG("har 1 act %d", i->action);*/
                har_act(scene->player1.har, i->action);
            } while((i = i->next));
            /*DEBUG("done");*/
        }
        i = p2;
        if (i) {
            do {
                /*DEBUG("har 2 act %d", i->action);*/
                har_act(scene->player2.har, i->action);
            } while((i = i->next));
            /*DEBUG("done");*/
        }
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
