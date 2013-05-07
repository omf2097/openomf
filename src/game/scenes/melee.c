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
int row_a, row_b; // 0 or 1
int column_a, column_b; // 0-4

struct players_t {
    sd_sprite *sprite;
    /*texture *tex;*/
};

struct players_t players[10];
struct players_t players_big[10];
struct players_t hars[10];

int player_id_a, player_id_b;

texture feh;
texture bleh;
texture select_hilight;
unsigned int ticks, hartick;
unsigned int pulsedir;

animationplayer harplayer_a;
animationplayer harplayer_b;

void melee_switch_animation(scene *scene, animationplayer *harplayer, int id) {
    animationplayer_free(harplayer);
    animationplayer_create(harplayer, id, array_get(&scene->animations, id));
    animationplayer_run(harplayer);
}

// Init menus
int melee_init(scene *scene) {
    char bitmap[51*36*4];
    memset(&bitmap, 255, 51*36*4);
    ticks = 0;
    pulsedir = 0;
    selection = 0;
    row_a = 0;
    column_a = 0;
    row_b = 0;
    column_b = 4;
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
            animationplayer_run(&harplayer_a);
            if (scene->player2.selectable) {
                animationplayer_run(&harplayer_b);
            }
        }
        if(harplayer_a.finished) {
            animationplayer_reset(&harplayer_a);
        }
        if (scene->player2.selectable && harplayer_b.finished) {
            animationplayer_reset(&harplayer_b);
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
                    player_id_a = 5*row_a + column_a;
                    player_id_b = 5*row_b + column_b;
                    selection = 1;
                } else {
                    har *h1 = malloc(sizeof(har));
                    har *h2 = malloc(sizeof(har));
                    har_load(h1, scene->bk->palettes[0], scene->bk->soundtable, 5*row_a+column_a, 60, 190, 1);
                    if (scene->player2.selectable) {
                        har_load(h2, scene->bk->palettes[0], scene->bk->soundtable, 5*row_b+column_b, 260, 190, -1);
                    } else {
                        // default to jaguar
                        har_load(h2, scene->bk->palettes[0], scene->bk->soundtable, HAR_JAGUAR, 260, 190, -1);
                    }
                    scene_set_player1_har(scene, h1);
                    scene_set_player2_har(scene, h2);
                    scene->next_id = SCENE_ARENA3;
                }
                break;
            case SDLK_LEFT:
                column_a--;
                if (column_a < 0) {
                    column_a = 0;
                }
                break;
            case SDLK_RIGHT:
                column_a++;
                if (column_a > 4) {
                    column_a = 4;
                }
                break;
            case SDLK_UP:
                row_a = 0;
                break;
            case SDLK_DOWN:
                row_a = 1;
                break;
            case SDLK_a:
                column_b--;
                if (column_b < 0) {
                    column_b = 0;
                }
                break;
            case SDLK_d:
                column_b++;
                if (column_b > 4) {
                    column_b = 4;
                }
                break;
            case SDLK_w:
                row_b = 0;
                break;
            case SDLK_s:
                row_b = 1;
                break;
        }
        if (selection == 1) {
            int har_animation_a = (5*row_a) + column_a + 18;
            if (harplayer_a.id != har_animation_a) {
                melee_switch_animation(scene, &harplayer_a, har_animation_a);
                // these are just guesses
                harplayer_a.x = 110;
                harplayer_a.y = 95;
                animationplayer_set_direction(&harplayer_a, 1);
            }
            if (scene->player2.selectable) {
                int har_animation_b = (5*row_b) + column_b + 18;
                if (harplayer_b.id != har_animation_b) {
                    melee_switch_animation(scene, &harplayer_b, har_animation_b);
                    // these are just guesses
                    harplayer_b.x = 320-110;
                    harplayer_b.y = 95;
                    animationplayer_set_direction(&harplayer_b, -1);
                }
            }
        }
        return 1;
    }
    return 0;
}

void melee_render(scene *scene) {
    animation *ani;
    int current_a = 5*row_a + column_a;
    int current_b = 5*row_b + column_b;

    if (selection == 0) {
        video_render_sprite_flip(&feh, 70, 0, BLEND_ALPHA, FLIP_NONE);
        video_render_sprite_flip(&bleh, 0, 62, BLEND_ALPHA, FLIP_NONE);
    }
    if (scene->player2.selectable && current_a == current_b) {
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), 250 - ticks , 0, 250 - ticks);
    } else {
        if (scene->player2.selectable) {
            video_render_char(&select_hilight, 11 + (62*column_b), 115 + (42*row_b), 0 , 0, 250 - ticks);
        }
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), 250 - ticks, 0, 0);
    }
    ani = array_get(&scene->animations, 5);
    if (scene->player2.selectable) {
        video_render_sprite_flip(array_get(&ani->sprites, 0), 254, 0, BLEND_ALPHA, FLIP_NONE);
    } else {
        video_render_sprite_flip(array_get(&ani->sprites, 1), 162, 0, BLEND_ALPHA, FLIP_NONE);
    }
    if (selection == 0) {
        for(int i = 0; i < 10; i++) {
            ani = array_get(&scene->animations, 3);

            video_render_sprite_flip(array_get(&ani->sprites, i), players[i].sprite->pos_x, players[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

            if (i == current_a) {
                ani = array_get(&scene->animations, 4);
                video_render_sprite_flip(array_get(&ani->sprites, i), players_big[i].sprite->pos_x, players_big[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
            }

            if (scene->player2.selectable && i == current_b) {
                ani = array_get(&scene->animations, 4);
                video_render_sprite_flip(array_get(&ani->sprites, i), 320-(players_big[i].sprite->img->w + players_big[i].sprite->pos_x), players_big[i].sprite->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
            }
        }
    } else {
        sd_sprite *sprite = scene->bk->anims[1]->animation->sprites[0];
        ani = array_get(&scene->animations, 1);
        video_render_sprite_flip(array_get(&ani->sprites, 0), sprite->pos_x, sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
        ani = array_get(&scene->animations, 4);
        video_render_sprite_flip(array_get(&ani->sprites, player_id_a), players_big[player_id_a].sprite->pos_x, players_big[player_id_a].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

        animationplayer_render(&harplayer_a);
        if (scene->player2.selectable) {
            video_render_sprite_flip(array_get(&ani->sprites, player_id_b), 320-(players_big[player_id_b].sprite->img->w + players_big[player_id_b].sprite->pos_x),
                    players_big[player_id_b].sprite->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
            animationplayer_render(&harplayer_b);
        }
    }

}

void melee_load(scene *scene) {
    scene->event = melee_event;
    scene->render = melee_render;
    scene->init = melee_init;
    scene->deinit = melee_deinit;
    scene->tick = melee_tick;
}

