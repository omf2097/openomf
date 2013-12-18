#include <stdlib.h>
#include <SDL2/SDL.h>
#include "utils/log.h"
#include "utils/random.h"
#include "video/video.h"
#include "resources/ids.h"
#include "game/text/text.h"
#include "game/text/languages.h"
#include "game/protos/scene.h"
#include "game/scenes/vs.h"
#include "game/menu/menu_background.h"
#include "game/game_state.h"
#include "controller/controller.h"
#include "controller/keyboard.h"

void cb_vs_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata);
void cb_vs_destroy_object(object *parent, int id, void *userdata);

typedef struct vs_local_t {
    texture player2_background;
    object player1_portrait;
    object player2_portrait;
    object player1_har;
    object player2_har;
    texture arena_select_bg;
    object arena_select;
    palette *player1_palette;
    palette *player2_palette;
    int arena;
} vs_local;

void cb_vs_spawn_object(object *parent, int id, vec2i pos, int g, void *userdata) {
    scene *s = (scene*)userdata;

    // Get next animation
    bk_info *info = bk_get_info(&s->bk_data, id);
    if(info != NULL) {
        object *obj = malloc(sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, vec2f_to_i(parent->pos)), vec2f_create(0,0));
        object_set_stl(obj, object_get_stl(parent));
        object_set_palette(obj, object_get_palette(parent), 0);
        object_set_animation(obj, &info->ani);
        object_set_spawn_cb(obj, cb_vs_spawn_object, userdata);
        object_set_destroy_cb(obj, cb_vs_destroy_object, userdata);
        game_state_add_object(parent->gs, obj, RENDER_LAYER_MIDDLE);
    }
}

void cb_vs_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_animation(parent->gs, id);
}


sd_rgba_image* sub_image(sd_vga_image *image, palette *pal, int x, int y, int w, int h) {
    sd_rgba_image *img = 0;
    sd_rgba_image *out = sd_rgba_image_create(w, h);
    img = sd_vga_image_decode(image, (sd_palette*)pal, -1);
    for(int i = y; i < y+h; i++) {
        for(int j = x; j < x+w; j++) {
            int offset = (i*image->w*4)+(j*4);
            int local_offset = ((i-y)*w*4)+((j-x)*4);
            out->data[local_offset]   = (char)img->data[offset];
            out->data[local_offset+1] = (char)img->data[offset+1];
            out->data[local_offset+2] = (char)img->data[offset+2];
            out->data[local_offset+3] = (char)img->data[offset+3];
        }
    }
    sd_rgba_image_delete(img);
    return out;
}

void vs_free(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    texture_free(&local->player2_background);
    texture_free(&local->arena_select_bg);
    object_free(&local->player1_portrait);
    object_free(&local->player2_portrait);
    object_free(&local->player1_har);
    object_free(&local->player2_har);
    if (player2->selectable) {
        object_free(&local->arena_select);
    }
    free(local->player1_palette);
    free(local->player2_palette);
    free(local);
}

void vs_handle_action(scene *scene, int action) {
    vs_local *local = scene_get_userdata(scene);
    switch (action) {
        case ACT_KICK:
        case ACT_PUNCH:
            game_state_set_next(scene->gs, SCENE_ARENA0+local->arena);
            break;
        case ACT_UP:
        case ACT_LEFT:
            if(game_state_get_player(scene->gs, 1)->selectable) {
                local->arena--;
                if (local->arena < 0) {
                    local->arena =4;
                }
                object_select_sprite(&local->arena_select, local->arena);
            }
            break;
        case ACT_DOWN:
        case ACT_RIGHT:
            if(game_state_get_player(scene->gs, 1)->selectable) {
                local->arena++;
                if (local->arena > 4) {
                    local->arena = 0;
                }
                object_select_sprite(&local->arena_select, local->arena);
            }
            break;
    }
}

void vs_tick(scene *scene) {
    // TODO remove this?
}

int vs_event(scene *scene, SDL_Event *event) {
    if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_ESCAPE) {
        game_state_set_next(scene->gs, SCENE_MELEE);
        return 1;
    }
    ctrl_event *p1=NULL, *i;
    game_player *player1 = game_state_get_player(scene->gs, 0);
    controller_event(player1->ctrl, event, &p1);
    i = p1;
    if (i) {
        do {
            vs_handle_action(scene, i->action);
        } while((i = i->next));
        DEBUG("done");
    }
    controller_free_chain(p1);
    return 1;
}

void vs_render(scene *scene) {
    vs_local *local = scene_get_userdata(scene);

    // render the right side of the background
    video_render_sprite_flip(&local->player2_background, 160, 0, BLEND_ALPHA, FLIP_HORIZONTAL);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    // player 1 HAR
    object_render(&local->player1_har);

    // player 2 HAR
    object_render(&local->player2_har);

    // player 1 portrait
    object_render(&local->player1_portrait);

    // player 2 portrait
    object_render(&local->player2_portrait);


    if (player2->selectable) {
        // arena selection
        video_render_sprite_flip(&local->arena_select_bg, 55, 150, BLEND_ALPHA, FLIP_NONE);

        object_render(&local->arena_select);

        // arena name
        font_render_wrapped(&font_small, lang_get(56+local->arena), 59+72, 153, (211-72)-4, COLOR_GREEN);

        // arena description
        font_render_wrapped(&font_small, lang_get(66+local->arena), 59+72, 161, (211-72)-4, COLOR_GREEN);

    } else {
        font_render_wrapped(&font_small, lang_get(749+(11*player1->pilot_id)+player2->pilot_id), 59, 160, 150, COLOR_YELLOW);
        font_render_wrapped(&font_small, lang_get(870+(11*player2->pilot_id)+player1->pilot_id), 320-(59+150), 180, 150, COLOR_YELLOW);
    }
}

int vs_create(scene *scene) {
    // Init local data
    vs_local *local = malloc(sizeof(vs_local));
    scene_set_userdata(scene, local);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    animation *ani;

    palette *mpal = bk_get_palette(&scene->bk_data, 0);

    local->player1_palette = palette_copy(mpal);
    local->player2_palette = palette_copy(mpal);
    palette_set_player_color(local->player1_palette, player1->colors[2], 0);
    palette_set_player_color(local->player1_palette, player1->colors[1], 1);
    palette_set_player_color(local->player1_palette, player1->colors[0], 2);

    palette_set_player_color(local->player2_palette, player2->colors[2], 0);
    palette_set_player_color(local->player2_palette, player2->colors[1], 1);
    palette_set_player_color(local->player2_palette, player2->colors[0], 2);

    // HAR
    ani = &bk_get_info(&scene->bk_data, 5)->ani;
    object_create(&local->player1_har, scene->gs, vec2i_create(160,0), vec2f_create(0, 0));
    object_set_animation(&local->player1_har, ani);
    object_set_palette(&local->player1_har, local->player1_palette, 0);
    object_select_sprite(&local->player1_har, player1->har_id - HAR_JAGUAR);

    object_create(&local->player2_har, scene->gs, vec2i_create(160,0), vec2f_create(0, 0));
    object_set_animation(&local->player2_har, ani);
    object_set_palette(&local->player2_har, local->player2_palette, 0);
    object_select_sprite(&local->player2_har, player2->har_id - HAR_JAGUAR);
    object_set_direction(&local->player2_har, OBJECT_FACE_LEFT);

    // PLAYER
    ani = &bk_get_info(&scene->bk_data, 4)->ani;
    object_create(&local->player1_portrait, scene->gs, vec2i_create(-10,150), vec2f_create(0, 0));
    object_set_animation(&local->player1_portrait, ani);
    object_set_palette(&local->player1_portrait, mpal, 0);
    object_select_sprite(&local->player1_portrait, player1->pilot_id);

    object_create(&local->player2_portrait, scene->gs, vec2i_create(330,150), vec2f_create(0, 0));
    object_set_animation(&local->player2_portrait, ani);
    object_set_palette(&local->player2_portrait, mpal, 0);
    object_select_sprite(&local->player2_portrait, player2->pilot_id);
    object_set_direction(&local->player2_portrait, OBJECT_FACE_LEFT);

    // clone the left side of the background image
    sd_rgba_image * out = sub_image((sd_vga_image*)scene->bk_data.background.raw_sprite, bk_get_palette(&scene->bk_data, 0), 0, 0, 160, 200);

    if (player2->selectable) {
        // player1 gets to choose, start at arena
        local->arena = 0;
    } else {
        // pick a random arena for 1 player mode
        local->arena = rand_int(5); // srand was done in melee
    }

    //ARENA
    if (player2->selectable) {
        ani = &bk_get_info(&scene->bk_data, 3)->ani;
        object_create(&local->arena_select, scene->gs, vec2i_create(59,155), vec2f_create(0, 0));
        object_set_animation(&local->arena_select, ani);
        object_set_palette(&local->arena_select, mpal, 0);
        object_select_sprite(&local->arena_select, local->arena);
    }

    
    // SCIENTIST
    object *o_scientist = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 8)->ani;
    object_create(o_scientist, scene->gs, vec2i_create(320-114,118), vec2f_create(0, 0));
    object_set_animation(o_scientist, ani);
    object_set_palette(o_scientist, mpal, 0);
    object_select_sprite(o_scientist, 0);
    object_set_direction(o_scientist, OBJECT_FACE_LEFT);
    game_state_add_object(scene->gs, o_scientist, RENDER_LAYER_MIDDLE);

    // WELDER
    object *o_welder = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 7)->ani;
    object_create(o_welder, scene->gs, vec2i_create(90,80), vec2f_create(0, 0));
    object_set_animation(o_welder, ani);
    object_set_palette(o_welder, mpal, 0);
    object_select_sprite(o_welder, 0);
    object_set_spawn_cb(o_welder, cb_vs_spawn_object, (void*)scene);
    object_set_destroy_cb(o_welder, cb_vs_destroy_object, (void*)scene);
    game_state_add_object(scene->gs, o_welder, RENDER_LAYER_MIDDLE);

    // GANTRIES
    object *o_gantry_a = malloc(sizeof(object));
    ani = &bk_get_info(&scene->bk_data, 11)->ani;
    object_create(o_gantry_a, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(o_gantry_a, ani);
    object_set_palette(o_gantry_a, mpal, 0);
    object_select_sprite(o_gantry_a, 0);
    game_state_add_object(scene->gs, o_gantry_a, RENDER_LAYER_MIDDLE);

    object *o_gantry_b = malloc(sizeof(object));
    object_create(o_gantry_b, scene->gs, vec2i_create(0,0), vec2f_create(0, 0));
    object_set_animation(o_gantry_b, ani);
    object_set_palette(o_gantry_b, mpal, 0);
    object_select_sprite(o_gantry_b, 0);
    object_set_direction(o_gantry_b, OBJECT_FACE_LEFT);
    game_state_add_object(scene->gs, o_gantry_b, RENDER_LAYER_MIDDLE);

    // Background tex
    texture_create(&local->player2_background);
    texture_init(&local->player2_background, out->data, 160, 200);

    menu_background2_create(&local->arena_select_bg, 211, 50);
    sd_rgba_image_delete(out);

    // Callbacks
    scene_set_render_cb(scene, vs_render);
    scene_set_event_cb(scene, vs_event);
    scene_set_tick_cb(scene, vs_tick);
    scene_set_free_cb(scene, vs_free);
    return 0;
}

