#include <string.h>
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>
#include "utils/log.h"
#include "game/text/text.h"
#include "audio/music.h"
#include "video/video.h"
#include "game/settings.h"
#include "game/text/languages.h"
#include "game/scene.h"
#include "game/scenes/melee.h"
#include "game/menu/menu_background.h"

int selection; // 0 for player, 1 for HAR
int row_a, row_b; // 0 or 1
int column_a, column_b; // 0-4

struct players_t {
    sd_sprite *sprite;
};

struct players_t players[10];
struct players_t players_big[10];

int player_id_a, player_id_b;

texture feh;
texture bleh;
texture select_hilight;
texture harportraits[10];
unsigned int ticks, hartick;
unsigned int pulsedir;

animationplayer harplayer_a;
animationplayer harplayer_b;

void melee_switch_animation(scene *scene, animationplayer *harplayer, int id) {
    animationplayer_free(harplayer);
    animationplayer_create(harplayer, id, array_get(&scene->animations, id));
    animationplayer_run(harplayer);
}

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
                    // restorethe player selection
                    column_a = player_id_a % 5;
                    row_a = player_id_a / 5;
                    column_b = player_id_b % 5;
                    row_b = player_id_b / 5;

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
                    scene->player1.har_id = 5*row_a+column_a;
                    scene->player1.player_id = player_id_a;
                    if (scene->player2.selectable) {
                        scene->player2.har_id = 5*row_b+column_b;
                        scene->player2.player_id = player_id_b;
                    } else {
                        // default to jaguar
                        scene->player2.har_id = 0;
                        // default to shirro
                        scene->player2.player_id = 4;
                    }
                    scene->next_id = SCENE_VS;
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

void render_highlights(scene *scene) {
    if (scene->player2.selectable && row_a == row_b && column_a == column_b) {
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), 250 - ticks , 0, 250 - ticks);
    } else {
        if (scene->player2.selectable) {
            video_render_char(&select_hilight, 11 + (62*column_b), 115 + (42*row_b), 0 , 0, 250 - ticks);
        }
        video_render_char(&select_hilight, 11 + (62*column_a), 115 + (42*row_a), 250 - ticks, 0, 0);
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
        font_render_wrapped(&font_small, lang_get(135+current_a), 4, 66, 152, 0, 255, 0);

        if (scene->player2.selectable) {
            video_render_sprite_flip(&feh, 320-70-feh.w, 0, BLEND_ALPHA, FLIP_NONE);
            video_render_sprite_flip(&bleh, 320-bleh.w, 62, BLEND_ALPHA, FLIP_NONE);
            // player bio
            font_render_wrapped(&font_small, lang_get(135+current_b), 320-bleh.w+4, 66, 152, 0, 255, 0);
        } else {
            // 'choose your pilot'
            font_render_wrapped(&font_small, lang_get(187), 160, 97, 160, 0, 255, 0);
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
        font_render_wrapped(&font_small, lang_get(20+current_a), 0, 52, 66, 0, 0, 0);

        if (scene->player2.selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+current_b), 320-66, 52, 66, 0, 0, 0);
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
        font_render_wrapped(&font_small, lang_get(20+player_id_a), 0, 52, 66, 0, 0, 0);

        if (scene->player2.selectable) {
            // player 2 name
            font_render_wrapped(&font_small, lang_get(20+player_id_b), 320-66, 52, 66, 0, 0, 0);

            // currently selected player
            video_render_sprite_flip(array_get(&ani->sprites, player_id_b), 320-(players_big[player_id_b].sprite->img->w + players_big[player_id_b].sprite->pos_x),
                    players_big[player_id_b].sprite->pos_y, BLEND_ALPHA, FLIP_HORIZONTAL);
            // currently selected HAR
            video_render_sprite_flip(&harportraits[current_b], 11 + (62*column_b), 115 + (42*row_b), BLEND_ALPHA, FLIP_NONE);
            animationplayer_render(&harplayer_b);
        } else {
            // 'choose your HAR'
            font_render_wrapped(&font_small, lang_get(186), 160, 97, 160, 0, 255, 0);
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

