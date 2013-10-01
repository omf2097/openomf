#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

#include "video/texture.h"
#include "video/video.h"
#include "game/scenes/arena.h"
#include "game/scenes/progressbar.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "game/settings.h"
#include "game/objects/har.h"
#include "game/protos/object.h"
#include "game/score.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/menu/menu.h"
#include "game/menu/menu_background.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "controller/controller.h"
#include "controller/net_controller.h"
#include "resources/ids.h"
#include "utils/log.h"

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

    palette *player_palettes[2];
} arena_local;

// -------- Local callbacks --------

void game_menu_quit(component *c, void *userdata) {
    for(int i = 0; i < 2; i++) {
        controller *ctrl = game_player_get_ctrl(game_state_get_player(i));
        if(ctrl->type == CTRL_TYPE_NETWORK) {
            net_controller_free(ctrl);
        }
    }

    game_state_set_next(SCENE_MENU);
}

void game_menu_return(component *c, void *userdata) {
    arena_local *local = scene_get_userdata((scene*)userdata);
    local->menu_visible = 0;
}

void music_slide(component *c, void *userdata, int pos) {
    audio_set_volume(TYPE_MUSIC, pos/10.0f);
}

void sound_slide(component *c, void *userdata, int pos) {
    audio_set_volume(TYPE_EFFECT, pos/10.0f);
}

// -------- Scene callbacks --------

void arena_free(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(i);
        game_player_set_har(player, NULL);
        game_player_set_ctrl(player, NULL);
    }
    
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

    free(local->player_palettes[0]);
    free(local->player_palettes[1]);
    
    settings_save();
    
    free(local);
}

void arena_tick(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(0);
    game_player *player2 = game_state_get_player(1);
    
    // Handle scrolling score texts
    chr_score_tick(&local->player1_score);
    chr_score_tick(&local->player2_score);

    // Handle menu, if visible
    if(!local->menu_visible) {
        ctrl_event *p1 = NULL, *p2 = NULL, *i;
        if(controller_tick(player1->ctrl, &p1) ||
                controller_tick(player2->ctrl, &p2)) {
            // one of the controllers bailed

            if(player1->ctrl->type == CTRL_TYPE_NETWORK) {
                net_controller_free(player1->ctrl);
            }

            if(player2->ctrl->type == CTRL_TYPE_NETWORK) {
                net_controller_free(player2->ctrl);
            }
            /*scene->next_id = SCENE_MENU;*/
        }

        i = p1;
        if (i) {
            do {
                object_act(game_player_get_har(player1), i->action);
            } while((i = i->next));
        }
        controller_free_chain(p1);
        i = p2;
        if (i) {
            do {
                object_act(game_player_get_har(player2), i->action);
            } while((i = i->next));
        }
        controller_free_chain(p2);
        
        // Collision detections
        //har_collision_har(scene->player1.har, scene->player2.har);
        //har_collision_har(scene->player2.har, scene->player1.har);
        // XXX this can't go in har.c because of a typedef loop on OSX
        //har_collision_scene(scene->player1.har, scene);
        //har_collision_scene(scene->player2.har, scene);
        
        // Turn the HARs to face the enemy
        vec2i pos1, pos2;
        object *har1,*har2;
        har1 = game_player_get_har(game_state_get_player(0));
        har2 = game_player_get_har(game_state_get_player(1));
        pos1 = object_get_pos(har1);
        pos2 = object_get_pos(har2);
        if(pos1.x > pos2.x) {
            if(object_get_direction(har1) == OBJECT_FACE_RIGHT) {
                object_set_direction(har1, OBJECT_FACE_LEFT);
                object_set_direction(har2, OBJECT_FACE_RIGHT);
            }
        } else if(pos1.x < pos2.x) {
            if(object_get_direction(har1) == OBJECT_FACE_LEFT) {
                object_set_direction(har1, OBJECT_FACE_RIGHT);
                object_set_direction(har2, OBJECT_FACE_LEFT);
            }
        }
    }
}

int arena_event(scene *scene, SDL_Event *e) {
    arena_local *local = scene_get_userdata(scene);

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
        for(int i = 0; i < 2; i++) {
            ctrl_event *p = NULL, *n = NULL;
            game_player *player = game_state_get_player(i);
            controller_event(game_player_get_ctrl(player), e, &p);
            n = p;
            if(n) {
                do {
                    object_act(game_player_get_har(player), n->action);
                } while((n = n->next));
            }
            controller_free_chain(p);
        }
        return 0;
    }
    return 0;
}

void arena_render(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    
    // Set health bar
    game_player *player[2];
    object *obj[2];
    har *har[2];
    for(int i = 0; i < 2; i++) {
        player[i] = game_state_get_player(i);
        obj[i] = game_player_get_har(player[i]);
        har[i] = object_get_userdata(obj[i]);
    }
    if(obj[0] != NULL && obj[1] != NULL) {
        float p1_hp = (float)har[0]->health / (float)har[0]->health_max;
        float p2_hp = (float)har[1]->health / (float)har[1]->health_max;
        progressbar_set(&local->player1_health_bar, p1_hp * 100);
        progressbar_set(&local->player2_health_bar, p2_hp * 100);
        progressbar_render(&local->player1_health_bar);
        progressbar_render(&local->player2_health_bar);

        // Set endurance bar
        float p1_en = (float)har[0]->endurance / (float)har[0]->endurance_max;
        float p2_en = (float)har[1]->endurance / (float)har[1]->endurance_max;
        progressbar_set(&local->player1_endurance_bar, p1_en * 100);
        progressbar_set(&local->player2_endurance_bar, p2_en * 100);
        progressbar_render(&local->player1_endurance_bar);
        progressbar_render(&local->player2_endurance_bar);

        // Render HAR and pilot names
        font_render(&font_small, lang_get(player[0]->player_id+20), 5, 19, TEXT_COLOR);
        font_render(&font_small, lang_get((player[0]->har_id - HAR_JAGUAR)+31), 5, 26, TEXT_COLOR);
        int p2len = (strlen(lang_get(player[1]->player_id+20))-1) * font_small.w;
        int h2len = (strlen(lang_get((player[1]->har_id - HAR_JAGUAR)+31))-1) * font_small.w;
        font_render(&font_small, lang_get(player[1]->player_id+20), 315-p2len, 19, TEXT_COLOR);
        font_render(&font_small, lang_get((player[1]->har_id - HAR_JAGUAR)+31), 315-h2len, 26, TEXT_COLOR);
        
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
}

void arena_render_overlay(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    if(local->menu_visible) {
        menu_render(&local->game_menu);
        video_render_sprite(&local->tex, 10, 150, BLEND_ALPHA_FULL);
    }
}

int arena_create(scene *scene) {
    settings *setting;
    arena_local *local;
    
    // Load up settings
    setting = settings_get();
    
    // Handle music playback
    music_stop();
    switch (scene->bk_data.file_id) {
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
    scene_set_userdata(scene, local);

    // set up palettes
    palette *mpal = bk_get_palette(&scene->bk_data, 0);

    local->player_palettes[0] = palette_copy(mpal);
    local->player_palettes[1] = palette_copy(mpal);

    // Initial har data
    vec2i pos[2];
    vec2f vel[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(60, 190);
    pos[1] = vec2i_create(260, 190);
    vel[0] = vec2f_create(0,0);
    vel[1] = vec2f_create(0,0);

    // init HARs
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(i);
        object obj;

        // load the player's colors into the palette
        palette_set_player_color(local->player_palettes[i], 0, player->colors[2], 0);
        palette_set_player_color(local->player_palettes[i], 0, player->colors[1], 1);
        palette_set_player_color(local->player_palettes[i], 0, player->colors[0], 2);

        // Create object and specialize it as HAR.
        // Errors are unlikely here, but check anyway.
        object_create(&obj, pos[i], vel[i]);
        if(har_create(&obj, local->player_palettes[i], dir[i], player->har_id)) {
            return 1;
        }

        // Set HAR to controller and game_player
        game_state_add_object(&obj);

        // Get reference to har and set it to
        // TODO: FIX THIS UGLY, UGLY HACK
        object *m = game_state_get_latest_obj();
        game_player_set_har(player, m);
        game_player_get_ctrl(player)->har = m;
    }

    // remove the keyboard hooks
    // set up the magic HAR hooks
    /*
    game_player *_player[2];
    for(int i = 0; i < 2; i++) {
        _player[i] = game_state_get_player(i);
    }
    if(game_player_get_ctrl(_player[0])->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(game_player_get_ctrl(_player[1]));
        har_add_hook(
            game_player_get_har(_player[1]), 
            game_player_get_ctrl(_player[0])->har_hook, 
            game_player_get_ctrl(_player[0]));
    }
    if(game_player_get_ctrl(_player[1])->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(game_player_get_ctrl(_player[0]));
        har_add_hook(
            game_player_get_har(_player[0]), 
            game_player_get_ctrl(_player[1])->har_hook, 
            game_player_get_ctrl(_player[1]));
    }*/
    
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

    // Callbacks
    scene_set_render_cb(scene, arena_render);
    scene_set_event_cb(scene, arena_event);
    scene_set_free_cb(scene, arena_free);
    scene_set_tick_cb(scene, arena_tick);
    scene_set_render_overlay_cb(scene, arena_render_overlay);

    // All done!
    return 0;
}
