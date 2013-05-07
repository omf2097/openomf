#include <string.h>
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scene.h"
#include "game/scenes/melee.h"
#include "game/menu/menu_background.h"

int selection; // 0 for player, 1 for HAR
int row; // 0 or 1
int column; // 0-4

struct players_t {
    sd_sprite *sprite;
    /*texture *tex;*/
};

struct players_t players[10];
struct players_t players_big[10];
struct players_t hars[10];

int player_id;

texture feh;
texture bleh;
texture select_hilight;
unsigned int ticks, hartick;
unsigned int pulsedir;

animationplayer harplayer;

void melee_switch_animation(scene *scene, int id) {
    animationplayer_free(&harplayer);
    animationplayer_create(&harplayer, id, array_get(&scene->animations, id));
    animationplayer_set_direction(&harplayer, 1);
    animationplayer_run(&harplayer);
    // these are just guesses
    harplayer.x = 110;
    harplayer.y = 95;
}

// Init menus
int melee_init(scene *scene) {
    char bitmap[51*36*4];
    memset(&bitmap, 255, 51*36*4);
    ticks = 0;
    pulsedir = 0;
    selection = 0;
    row = 0;
    column=0;
    for(int i = 0; i < 10; i++) {
        players[i].sprite = scene->bk->anims[3]->animation->sprites[i];
        DEBUG("found sprite %d x %d at %d, %d", players[i].sprite->img->w, players[i].sprite->img->h, players[i].sprite->pos_x, players[i].sprite->pos_y);
        players_big[i].sprite = scene->bk->anims[4]->animation->sprites[i];
    }
    menu_background_create(&feh, 90, 61);
    menu_background_create(&bleh, 160, 43);
    texture_create(&select_hilight, bitmap, 51, 36);

    // All done
    return 0;
}

void melee_deinit(scene *scene) {
}

void melee_tick(scene *scene) {
    if(!pulsedir) {
        ticks++;
    } else {
        ticks--;
    }
    if(ticks > 120) {
        pulsedir = 1;
    }
    if(ticks == 0) {
        pulsedir = 0;
    }
    hartick++;
    if (selection == 1) {
        if(hartick > 10) {
            hartick = 0;
            animationplayer_run(&harplayer);
        }
        if(harplayer.finished) {
            animationplayer_reset(&harplayer);
        }
    }
}

int melee_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_ESCAPE:
                if (selection == 1) {
                    selection = 0;
                } else {
                    scene->next_id = SCENE_MENU;
                }
                break;
            case SDLK_RETURN:
                if (selection == 0) {
                    player_id = 5*row + column;
                    selection = 1;
                } else {
                    har *h1 = malloc(sizeof(har));
                    har *h2 = malloc(sizeof(har));
                    har_load(h1, scene->bk->palettes[0], scene->bk->soundtable, 5*row+column, 60, 190, 1);
                    har_load(h2, scene->bk->palettes[0], scene->bk->soundtable, 0, 260, 190, -1);
                    scene_set_player1_har(scene, h1);
                    scene_set_player2_har(scene, h2);
                    scene->next_id = SCENE_ARENA3;
                }
                break;
            case SDLK_LEFT:
                column--;
                if (column < 0) {
                    column = 0;
                }
                break;
            case SDLK_RIGHT:
                column++;
                if (column > 4) {
                    column = 4;
                }
                break;
            case SDLK_UP:
                row = 0;
                break;
            case SDLK_DOWN:
                row = 1;
                break;
        }
        if (selection == 1) {
            int har_animation = (5*row) + column + 18;
            if (harplayer.id != har_animation) {
                melee_switch_animation(scene, har_animation);
            }
        }
        return 1;
    }
    return 0;
}

void melee_render(scene *scene) {
    animation *ani;
    int current = 5*row + column;

    if (selection == 0) {
        video_render_sprite_flip(&feh, 70, 0, BLEND_ALPHA, FLIP_NONE);
        video_render_sprite_flip(&bleh, 0, 62, BLEND_ALPHA, FLIP_NONE);
    }
    video_render_char(&select_hilight, 11 + (62*column), 115 + (42*row), 250 - ticks , 0, 0);
    ani = array_get(&scene->animations, 5);
    video_render_sprite_flip(array_get(&ani->sprites, 1), 162, 0, BLEND_ALPHA, FLIP_NONE);
    if (selection == 0) {
        for(int i = 0; i < 10; i++) {
            ani = array_get(&scene->animations, 3);

            if (i == current) {
                video_render_sprite_flip(array_get(&ani->sprites, i), players[i].sprite->pos_x, players[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
                ani = array_get(&scene->animations, 4);
                video_render_sprite_flip(array_get(&ani->sprites, i), players_big[i].sprite->pos_x, players_big[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
            } else {
                video_render_sprite_flip(array_get(&ani->sprites, i), players[i].sprite->pos_x, players[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
            }

        }
    } else {
        sd_sprite *sprite = scene->bk->anims[1]->animation->sprites[0];
        ani = array_get(&scene->animations, 1);
        video_render_sprite_flip(array_get(&ani->sprites, 0), sprite->pos_x, sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
        ani = array_get(&scene->animations, 4);
        video_render_sprite_flip(array_get(&ani->sprites, player_id), players_big[player_id].sprite->pos_x, players_big[player_id].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

        animationplayer_render(&harplayer);
    }

}

void melee_load(scene *scene) {
    scene->event = melee_event;
    scene->render = melee_render;
    scene->init = melee_init;
    scene->deinit = melee_deinit;
    scene->tick = melee_tick;
}

