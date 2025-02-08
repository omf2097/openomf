#include <stdlib.h>

#include "audio/audio.h"
#include "game/game_state.h"
#include "game/gui/text_render.h"
#include "game/scenes/cutscene.h"
#include "resources/ids.h"
#include "resources/languages.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "video/video.h"

#define END_TEXT 992
#define END1_TEXT 993
#define END2_TEXT 1003

typedef struct cutscene_local {
    char *text;
    char *current;
    size_t len;
    int pos;
    int text_x;
    int text_y;
    int text_width;
    text_settings text_conf;
} cutscene_local;

static int cutscene_next_scene(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    switch(scene->id) {
        case SCENE_END:
            return SCENE_END1;
        case SCENE_END1:
            return SCENE_END2;
        case SCENE_END2:
            return SCENE_SCOREBOARD;
        default:
            if(player1->chr) {
                return SCENE_VS;
            }
            return SCENE_NONE;
    }
}

static void cutscene_input_tick(scene *scene) {
    cutscene_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    ctrl_event *p1 = NULL, *i;
    game_state_menu_poll(scene->gs, &p1);

    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH) {

                    if(player1->chr && player1->chr->cutscene_text[local->pos + 1]) {
                        local->pos++;
                        local->current = player1->chr->cutscene_text[local->pos];
                    } else if(!player1->chr && strlen(local->current) + local->pos < local->len) {
                        local->pos += strlen(local->current) + 1;
                        local->current += strlen(local->current) + 1;
                        char *p;
                        if((p = strchr(local->current, '\n'))) {
                            // null out the byte
                            *p = '\0';
                        }
                    } else {
                        game_state_set_next(scene->gs, cutscene_next_scene(scene));
                    }
                }
            }
        } while((i = i->next) != NULL);
    }
    controller_free_chain(p1);
}

static void cutscene_render_overlay(scene *scene) {
    cutscene_local *local = scene_get_userdata(scene);
    text_render(&local->text_conf, TEXT_DEFAULT, local->text_x, local->text_y, local->text_width, 200, local->current);
}

static void cutscene_free(scene *scene) {
    cutscene_local *local = scene_get_userdata(scene);
    omf_free(local->text);
    omf_free(local);
    scene_set_userdata(scene, local);
}

static void cutscene_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    if(scene->id == SCENE_END || scene->id == SCENE_END1) {
        if(id == 1) {
            *m_load = 1;
        }
    } else if(scene->id == SCENE_END2 && (id == 1 || id == 11)) {
        *m_load = 1;
    }
}

static void cutscene_spawn_random(scene *scene) {
    iterator it;
    hashmap_iter_begin(&scene->bk_data->infos, &it);
    hashmap_pair *pair = NULL;

    foreach(it, pair) {
        bk_info *info = (bk_info *)pair->value;
        if(info->probability > 1) {
            if(random_int(&scene->gs->rand, info->probability) != 1) {
                continue;
            }
            object *obj = omf_calloc(1, sizeof(object));
            object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0, 0));
            object_set_stl(obj, scene->bk_data->sound_translation_table);
            object_set_animation(obj, &info->ani);

            // If there was already playing instance, free the object.
            if(game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM, 1, 0) == 1) {
                object_free(obj);
                omf_free(obj);
            }
        }
    }
}

static void cutscene_dynamic_tick(scene *scene, int paused) {
    cutscene_spawn_random(scene);
}

int cutscene_create(scene *scene) {
    cutscene_local *local = omf_calloc(1, sizeof(cutscene_local));
    text_defaults(&local->text_conf);
    local->text_conf.halign = TEXT_CENTER;
    local->text_conf.font = FONT_SMALL;

    game_player *p1 = game_state_get_player(scene->gs, 0);

    const char *text = "";
    switch(scene->id) {
        case SCENE_END:
            audio_play_music(PSM_END);
            text = lang_get(END_TEXT);
            local->text_x = 10;
            local->text_y = 5;
            local->text_width = 300;
            local->text_conf.cforeground = 0xF8;
            break;

        case SCENE_END1:
            text = lang_get(END1_TEXT + p1->pilot->pilot_id);
            local->text_x = 10;
            local->text_y = 157;
            local->text_width = 300;
            local->text_conf.cforeground = 0xFD;

            // Pilot face
            animation *ani = &bk_get_info(scene->bk_data, 3)->ani;
            object *obj = omf_calloc(1, sizeof(object));
            object_create(obj, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(obj, ani);
            object_select_sprite(obj, p1->pilot->pilot_id);
            object_set_halt(obj, 1);
            game_state_add_object(scene->gs, obj, RENDER_LAYER_TOP, 0, 0);

            // Face effects
            ani = &bk_get_info(scene->bk_data, 10 + p1->pilot->pilot_id)->ani;
            obj = omf_calloc(1, sizeof(object));
            object_create(obj, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(obj, ani);
            game_state_add_object(scene->gs, obj, RENDER_LAYER_TOP, 0, 0);
            break;

        case SCENE_END2:
            text = lang_get(END2_TEXT + p1->pilot->pilot_id);
            local->text_x = 10;
            local->text_y = 160;
            local->text_width = 300;
            local->text_conf.cforeground = 0xF8;
            break;

        case SCENE_NORTHAM:
        case SCENE_KATUSHAI:
        case SCENE_WAR:
            // Load colors for the HAR -- note that cutscenes use an expanded HAR color slides.
            // World championship does not use these.
            palette_set_player_expanded_color(p1->chr->pilot.color_3, PRIMARY);
            palette_set_player_expanded_color(p1->chr->pilot.color_2, SECONDARY);
            palette_set_player_expanded_color(p1->chr->pilot.color_1, TERTIARY);

            // Fall through!
        case SCENE_WORLD:
            audio_play_music(PSM_END);

            // load all the animations, in order
            // including the one for our HAR
            for(int i = 0; i < 256; i++) {
                if(i >= 10 && i <= 20 && i != 10 + p1->pilot->har_id) {
                    continue;
                }
                bk_info *bki = bk_get_info(scene->bk_data, i);
                if(bki) {
                    ani = &bki->ani;
                    obj = omf_calloc(1, sizeof(object));
                    object_create(obj, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
                    object_set_stl(obj, scene->bk_data->sound_translation_table);
                    object_set_animation(obj, ani);
                    game_state_add_object(scene->gs, obj, RENDER_LAYER_TOP, 0, 0);
                }
            }
            local->text_x = 10;
            local->text_y = 160;
            local->text_width = 300;
            local->text_conf.cforeground = 0xF8;
            break;
    }

    if(p1->chr) {
        local->pos = 0;
        local->current = p1->chr->cutscene_text[local->pos];
    } else {
        size_t text_len = strlen(text);
        local->len = text_len - 1;
        local->pos = 0;
        local->text = omf_strdup(text);
        local->current = local->text;

        char *p;
        if((p = strchr(local->text, '\n'))) {
            // null out the byte
            *p = '\0';
        }
    }

    // Callbacks
    scene_set_userdata(scene, local);
    scene_set_free_cb(scene, cutscene_free);
    scene_set_input_poll_cb(scene, cutscene_input_tick);
    scene_set_startup_cb(scene, cutscene_startup);
    scene_set_render_overlay_cb(scene, cutscene_render_overlay);
    scene_set_dynamic_tick_cb(scene, cutscene_dynamic_tick);

    return 0;
}
