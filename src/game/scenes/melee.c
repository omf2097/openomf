#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/scene.h"
#include "game/scenes/melee.h"
#include "game/scenes/progressbar.h"
#include "game/menu/menu_background.h"

#define MAX_STAT 20

int selection; // 0 for player, 1 for HAR
int row_a, row_b; // 0 or 1
int column_a, column_b; // 0-4
int done_a, done_b; // 0-1

struct players_t {
    sd_sprite *sprite;
};

struct pilot_t {
    int power, agility, endurance;
};

struct pilot_t pilots[10] = {
    {5,16,9},
    {13,9,8},
    {7,20,4},
    {9,7,15},
    {20,1,8},
    {9,10,11},
    {10,1,20},
    {7,10,13},
    {14,8,8},
    {14,4,12}
};

struct players_t players[10];
struct players_t players_big[10];

progress_bar bar_power[2], bar_agility[2], bar_endurance[2];

int player_id_a, player_id_b;

texture feh;
texture bleh;
texture select_hilight;
texture harportraits[10];
unsigned int ticks, hartick;
unsigned int pulsedir;

animationplayer harplayer_a;
animationplayer harplayer_b;

void refresh_pilot_stats();

void melee_switch_animation(scene *scene, animationplayer *harplayer, int id, int x, int y) {
    object *obj = malloc(sizeof(object));
    object_create(obj, global_space, x, y, 0, 0, 1.0f, 1.0f, 0.0f);
    object_set_gravity(obj, 0.0f);
    animationplayer_free(harplayer);
    animationplayer_create(harplayer, id, array_get(&scene->animations, id), obj);
    animationplayer_set_repeat(harplayer, 1);
    animationplayer_run(harplayer);
}

void handle_action(scene *scene, int player, int action);

// extract part of a sprite as a new sprite
// we need this because the HAR portraits are one single sprite, unlike the player portraits
// so we need to chunk them up into individual sprites and strip out the black background
sd_rgba_image* sub_sprite(sd_sprite *sprite, sd_palette *pal, int x, int y, int w, int h) {
    sd_rgba_image *img = 0;
    sd_rgba_image *out = sd_rgba_image_create(w, h);
    img = sd_sprite_image_decode(sprite->img, pal, -1);
    for(int i = y; i < y+h; i++) {
        for(int j = x; j < x+w; j++) {
            int offset = (i*sprite->img->w*4)+(j*4);
            int local_offset = ((i-y)*w*4)+((j-x)*4);
            out->data[local_offset]   = (char)img->data[offset];
            out->data[local_offset+1] = (char)img->data[offset+1];
            out->data[local_offset+2] = (char)img->data[offset+2];
            if (!out->data[local_offset] && !out->data[local_offset+1] && !out->data[local_offset+2]) {
                // all three colors are black, set the pixel to be transparent!
                out->data[local_offset+3] = 0;
            } else {
                out->data[local_offset+3] = (char)img->data[offset+3];
            }
        }
    }
    sd_rgba_image_delete(img);
    return out;
}

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
    done_a = 0;
    done_b = 0;
    for(int i = 0; i < 10; i++) {
        players[i].sprite = scene->bk->anims[3]->animation->sprites[i];
        DEBUG("found sprite %d x %d at %d, %d", players[i].sprite->img->w, players[i].sprite->img->h, players[i].sprite->pos_x, players[i].sprite->pos_y);
        players_big[i].sprite = scene->bk->anims[4]->animation->sprites[i];

        int row = i / 5;
        int col = i % 5;
        sd_rgba_image * out = sub_sprite(scene->bk->anims[1]->animation->sprites[0], scene->bk->palettes[0], (62*col), (42*row), 51, 36);
        texture_create(&harportraits[i], out->data, 51, 36);
        sd_rgba_image_delete(out);
    }
    menu_background2_create(&feh, 90, 61);
    menu_background2_create(&bleh, 160, 43);
    texture_create(&select_hilight, bitmap, 51, 36);

    // set up the magic controller hooks
    if (scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {
        controller_add_hook(scene->player2.ctrl, scene->player1.ctrl, scene->player1.ctrl->controller_hook);
    }

    if (scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {
        controller_add_hook(scene->player1.ctrl, scene->player2.ctrl, scene->player2.ctrl->controller_hook);
    }


    const color bar_color = color_create(0, 190, 0, 255);
    const color bar_bg_color = color_create(80, 220, 80, 0);
    const color bar_border_color = color_create(0, 96, 0, 255);
    const color bar_top_left_border_color = color_create(0, 255, 0, 255);
    const color bar_bottom_right_border_color = color_create(0, 125, 0, 255);
    progressbar_create(&bar_power[0],     74, 12, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&bar_agility[0],   74, 30, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&bar_endurance[0], 74, 48, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&bar_power[1],     320-66-feh.w, 12, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&bar_agility[1],   320-66-feh.w, 30, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    progressbar_create(&bar_endurance[1], 320-66-feh.w, 48, 20*4, 8, bar_border_color, bar_border_color, bar_bg_color, bar_top_left_border_color, bar_bottom_right_border_color, bar_color, PROGRESSBAR_LEFT);
    for(int i = 0;i < 2;i++) {
        progressbar_set(&bar_power[i], 50);
        progressbar_set(&bar_agility[i], 50);
        progressbar_set(&bar_endurance[i], 50);
    }
    refresh_pilot_stats();

    memset(&harplayer_a, 0, sizeof(harplayer_a));
    memset(&harplayer_b, 0, sizeof(harplayer_b));

    // All done
    return 0;
}

void melee_deinit(scene *scene) {
    animationplayer_free(&harplayer_a);
    animationplayer_free(&harplayer_b);
    
    for(int i = 0; i < 10; i++) {
        texture_free(&harportraits[i]);
    }
    texture_free(&feh);
    texture_free(&bleh);
    texture_free(&select_hilight);
    for(int i = 0;i < 2;i++) {
        progressbar_free(&bar_power[i]);
        progressbar_free(&bar_agility[i]);
        progressbar_free(&bar_endurance[i]);
    }
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
    }

    ctrl_event *p1 = NULL, *p2 = NULL, *i;
    if(controller_tick(scene->player1.ctrl, &p1) ||
            controller_tick(scene->player2.ctrl, &p2)) {
        // one of the controllers bailed

        /*if(scene->player1.ctrl->type == CTRL_TYPE_NETWORK) {*/
            /*net_controller_free(scene->player1.ctrl);*/
        /*}*/

        /*if(scene->player2.ctrl->type == CTRL_TYPE_NETWORK) {*/
            /*net_controller_free(scene->player2.ctrl);*/
        /*}*/
        scene->next_id = SCENE_MENU;
    }
    i = p1;
    if (i) {
        do {
            handle_action(scene, 1, i->action);
        } while((i = i->next));
        DEBUG("done");
    }
    i = p2;
    if (i) {
        do {
            handle_action(scene, 2, i->action);
        } while((i = i->next));
        DEBUG("done");
    }
}

void refresh_pilot_stats() {
    int current_a = 5*row_a + column_a;
    int current_b = 5*row_b + column_b;
    progressbar_set(&bar_power[0], (pilots[current_a].power*100)/MAX_STAT);
    progressbar_set(&bar_agility[0], (pilots[current_a].agility*100)/MAX_STAT);
    progressbar_set(&bar_endurance[0], (pilots[current_a].endurance*100)/MAX_STAT);
    progressbar_set(&bar_power[1], (pilots[current_b].power*100)/MAX_STAT);
    progressbar_set(&bar_agility[1], (pilots[current_b].agility*100)/MAX_STAT);
    progressbar_set(&bar_endurance[1], (pilots[current_b].endurance*100)/MAX_STAT);
}

void handle_action(scene *scene, int player, int action) {
    int *row, *column, *done;
    if (player == 1) {
        DEBUG("event for player 1");
        row = &row_a;
        column = &column_a;
        done = &done_a;
    } else {
        DEBUG("event for player 2");
        row = &row_b;
        column = &column_b;
        done = &done_b;
    }

    if (*done) {
        return;
    }

    switch (action) {
        case ACT_LEFT:
            (*column)--;
            if (*column < 0) {
                *column = 4;
            }
            break;
        case ACT_RIGHT:
            (*column)++;
            if (*column > 4) {
                *column = 0;
            }
            break;
        case ACT_UP:
        case ACT_DOWN:
            *row = *row == 0 ? 1 : 0;
            break;
        case ACT_KICK:
        case ACT_PUNCH:
            *done = 1;
            if (done_a && (done_b || !scene->player2.selectable)) {
                done_a = 0;
                done_b = 0;
                if (selection == 0) {
                    selection = 1;
                    player_id_a = 5*row_a + column_a;
                    player_id_b = 5*row_b + column_b;
                } else {
                    scene->player1.har_id = 5*row_a+column_a;
                    scene->player1.player_id = player_id_a;
                    if (scene->player2.selectable) {
                        scene->player2.har_id = 5*row_b+column_b;
                        scene->player2.player_id = player_id_b;
                    } else {
                        // randomly pick opponent and HAR
                        srand(time(NULL));
                        scene->player2.har_id = rand() % 10;
                        int i;
                        while((i = rand() % 10) == player_id_a) {}
                        scene->player2.player_id = i;
                    }
                    scene->next_id = SCENE_VS;
                }
            }
            break;
    }

    refresh_pilot_stats();
    if (selection == 1) {
        int har_animation_a = (5*row_a) + column_a + 18;
        if (harplayer_a.id != har_animation_a) {
            melee_switch_animation(scene, &harplayer_a, har_animation_a, 110, 95);
            animationplayer_set_direction(&harplayer_a, 1);
        }
        if (scene->player2.selectable) {
            int har_animation_b = (5*row_b) + column_b + 18;
            if (harplayer_b.id != har_animation_b) {
                melee_switch_animation(scene, &harplayer_b, har_animation_b, 210, 95);
                animationplayer_set_direction(&harplayer_b, -1);
            }
        }
    }
}

int melee_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
                if (selection == 1) {
                    // restore the player selection
                    column_a = player_id_a % 5;
                    row_a = player_id_a / 5;
                    column_b = player_id_b % 5;
                    row_b = player_id_b / 5;

                    selection = 0;
                    done_a = 0;
                    done_b = 0;
                } else {
                    scene->next_id = SCENE_MENU;
                }
            } else {
                ctrl_event *p1=NULL, *p2 = NULL, *i;
                controller_event(scene->player1.ctrl, event, &p1);
                controller_event(scene->player2.ctrl, event, &p2);
                i = p1;
                if (i) {
                    do {
                        handle_action(scene, 1, i->action);
                    } while((i = i->next));
                    DEBUG("done");
                }
                i = p2;
                if (i) {
                    do {
                        handle_action(scene, 2, i->action);
                    } while((i = i->next));
                    DEBUG("done");
                }
            }
    }
    return 0;
}

void render_highlights(scene *scene) {
    int trans;
    if (scene->player2.selectable && row_a == row_b && column_a == column_b) {
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), 250 - ticks , 0, 250 - ticks);
    } else {
        if (scene->player2.selectable) {
            if (done_b) {
                trans = 250;
            } else {
                trans = 250 - ticks;
            }
            video_render_char(&select_hilight, 11 + (62*column_b), 115 + (42*row_b), 0 , 0, trans);
        }
        if (done_a) {
                trans = 250;
            } else {
                trans = 250 - ticks;
            }
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), trans, 0, 0);
    }
}

void melee_render(scene *scene) {
    animation *ani;
    int current_a = 5*row_a + column_a;
    int current_b = 5*row_b + column_b;


    if (selection == 0) {
        video_render_sprite_flip(&feh, 70, 0, BLEND_ALPHA, FLIP_NONE);
        video_render_sprite_flip(&bleh, 0, 62, BLEND_ALPHA, FLIP_NONE);

        // player bio
        font_render_wrapped(&font_small, lang_get(135+current_a), 4, 66, 152, COLOR_GREEN);
        // player stats
        font_render(&font_small, lang_get(216), 74+27, 4, COLOR_GREEN);
        font_render(&font_small, lang_get(217), 74+19, 22, COLOR_GREEN);
        font_render(&font_small, lang_get(218), 74+12, 40, COLOR_GREEN);
        progressbar_render(&bar_power[0]);
        progressbar_render(&bar_agility[0]);
        progressbar_render(&bar_endurance[0]);

        if (scene->player2.selectable) {
            video_render_sprite_flip(&feh, 320-70-feh.w, 0, BLEND_ALPHA, FLIP_NONE);
            video_render_sprite_flip(&bleh, 320-bleh.w, 62, BLEND_ALPHA, FLIP_NONE);
            // player bio
            font_render_wrapped(&font_small, lang_get(135+current_b), 320-bleh.w+4, 66, 152, COLOR_GREEN);
            // player stats
            font_render(&font_small, lang_get(216), 320-66-feh.w+27, 4, COLOR_GREEN);
            font_render(&font_small, lang_get(217), 320-66-feh.w+19, 22, COLOR_GREEN);
            font_render(&font_small, lang_get(218), 320-66-feh.w+12, 40, COLOR_GREEN);
            progressbar_render(&bar_power[1]);
            progressbar_render(&bar_agility[1]);
            progressbar_render(&bar_endurance[1]);
        } else {
            // 'choose your pilot'
            font_render_wrapped(&font_small, lang_get(187), 160, 97, 160, COLOR_GREEN);
        }
    }
    ani = array_get(&scene->animations, 5);

    if (scene->player2.selectable) {
        video_render_sprite_flip(array_get(&ani->sprites, 0), 254, 0, BLEND_ALPHA, FLIP_NONE);
    } else {
        video_render_sprite_flip(array_get(&ani->sprites, 1), 162, 0, BLEND_ALPHA, FLIP_NONE);
    }
    if (selection == 0) {
        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+current_a), 0, 52, 66, COLOR_BLACK);

        if (scene->player2.selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+current_b), 320-66, 52, 66, COLOR_BLACK);
        }

        render_highlights(scene);
        for(int i = 0; i < 10; i++) {
            ani = array_get(&scene->animations, 3);

            video_render_sprite_flip(array_get(&ani->sprites, i), players[i].sprite->pos_x, players[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

            if (i == current_a) {
                // render the big portrait
                ani = array_get(&scene->animations, 4);
                video_render_sprite_flip(array_get(&ani->sprites, i), players_big[i].sprite->pos_x, players_big[i].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);
            }

            if (scene->player2.selectable && i == current_b) {
                // render the big portrait
                ani = array_get(&scene->animations, 4);
                video_render_sprite_flip(array_get(&ani->sprites, i), 320-(players_big[i].sprite->img->w + players_big[i].sprite->pos_x), players_big[i].sprite->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
            }
        }
    } else {
        // render the stupid unselected HAR portraits before anything
        // so we can render anything else on top of them
        sd_sprite *sprite = scene->bk->anims[1]->animation->sprites[0];
        ani = array_get(&scene->animations, 1);
        video_render_sprite_flip(array_get(&ani->sprites, 0), sprite->pos_x, sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

        render_highlights(scene);

        // currently selected player
        ani = array_get(&scene->animations, 4);
        video_render_sprite_flip(array_get(&ani->sprites, player_id_a), players_big[player_id_a].sprite->pos_x, players_big[player_id_a].sprite->pos_y, BLEND_ALPHA, FLIP_NONE);

        //currently selected HAR
        video_render_sprite_flip(&harportraits[current_a], 11 + (62*column_a), 115 + (42*row_a), BLEND_ALPHA, FLIP_NONE);
        animationplayer_render(&harplayer_a);

        // player 1 name
        font_render_wrapped(&font_small, lang_get(20+player_id_a), 0, 52, 66, COLOR_BLACK);

        if (scene->player2.selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+player_id_b), 320-66, 52, 66, COLOR_BLACK);

            // currently selected player
            video_render_sprite_flip(array_get(&ani->sprites, player_id_b), 320-(players_big[player_id_b].sprite->img->w + players_big[player_id_b].sprite->pos_x),
                    players_big[player_id_b].sprite->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
            // currently selected HAR
            video_render_sprite_flip(&harportraits[current_b], 11 + (62*column_b), 115 + (42*row_b), BLEND_ALPHA, FLIP_NONE);
            animationplayer_render(&harplayer_b);
        } else {
            // 'choose your HAR'
            font_render_wrapped(&font_small, lang_get(186), 160, 97, 160, COLOR_GREEN);
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

