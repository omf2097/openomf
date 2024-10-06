#include "game/scenes/vs.h"
#include "controller/controller.h"
#include "game/game_state.h"
#include "game/gui/dialog.h"
#include "game/gui/menu_background.h"
#include "game/gui/text_render.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "video/video.h"
#include <stdio.h>
#include <stdlib.h>

void cb_vs_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t flags, int s, int g, void *userdata);
void cb_vs_destroy_object(object *parent, int id, void *userdata);

typedef struct vs_local_t {
    int arena_select_obj_id;
    surface arena_select_bg;
    int arena;
    char vs_str[128];
    dialog quit_dialog;
    dialog too_pathetic_dialog;
} vs_local;

int vs_is_netplay(scene *scene) {
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK ||
       game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
        return 1;
    }
    return 0;
}

int vs_is_singleplayer(scene *scene) {
    if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI) {
        return 1;
    }
    return 0;
}

// Even indexes go to the left, odd to the right.
// Welder does an additionale roll for the 3 places on the torso.
vec2i spawn_position(int index, int scientist) {
    switch(index) {
        case 0:
            // top left gantry
            if(scientist) {
                return vec2i_create(90, 80);
            }
            switch(rand_int(3)) {
                case 0:
                    // middle
                    return vec2i_create(90, 80);
                case 1:
                    // left arm
                    return vec2i_create(30, 80);
                case 2:
                    // right arm
                    return vec2i_create(120, 80);
            }
            break;
        case 1:
            // top right gantry
            if(scientist) {
                return vec2i_create(230, 80);
            }
            switch(rand_int(3)) {
                case 0:
                    // middle
                    return vec2i_create(230, 80);
                case 1:
                    // left arm
                    return vec2i_create(200, 80);
                case 2:
                    // right arm
                    return vec2i_create(260, 80);
            }
            break;
        case 2:
            // middle left gantry
            return vec2i_create(90, 118);
        case 3:
            // middle right gantry
            return vec2i_create(230, 118);
            // return vec2i_create(280,118);
        // only welder can use the following
        case 4:
            // bottom left gantry
            return vec2i_create(90, 150);
        case 5:
            // bottom right gantry
            return vec2i_create(230, 150);
        default:
            break;
    }
    // default
    return vec2i_create(160, 200);
}

void cb_vs_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t flags, int s, int g, void *userdata) {
    scene *sc = (scene *)userdata;

    // Get next animation
    bk_info *info = bk_get_info(sc->bk_data, id);
    if(info != NULL) {
        object *obj = omf_calloc(1, sizeof(object));
        object_create(obj, parent->gs, vec2i_add(pos, vec2f_to_i(parent->pos)), vel);
        object_set_stl(obj, object_get_stl(parent));
        object_set_animation(obj, &info->ani);
        object_set_spawn_cb(obj, cb_vs_spawn_object, userdata);
        object_set_destroy_cb(obj, cb_vs_destroy_object, userdata);
        game_state_add_object(parent->gs, obj, RENDER_LAYER_MIDDLE, 0, 0);
    }
}

void cb_vs_destroy_object(object *parent, int id, void *userdata) {
    game_state_del_animation(parent->gs, id);
}

void vs_free(scene *scene) {
    vs_local *local = scene_get_userdata(scene);

    dialog_free(&local->quit_dialog);
    dialog_free(&local->too_pathetic_dialog);
    surface_free(&local->arena_select_bg);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void vs_handle_action(scene *scene, int action) {
    vs_local *local = scene_get_userdata(scene);
    if(dialog_is_visible(&local->too_pathetic_dialog)) {
        dialog_event(&local->too_pathetic_dialog, action);
    } else if(dialog_is_visible(&local->quit_dialog)) {
        dialog_event(&local->quit_dialog, action);
    } else {
        switch(action) {
            case ACT_KICK:
            case ACT_PUNCH:
                if(game_state_get_player(scene->gs, 1)->pilot) {
                    game_state_set_next(scene->gs, SCENE_ARENA0 + local->arena);
                    if(vs_is_netplay(scene)) {
                        game_player *player2 = game_state_get_player(scene->gs, 1);
                        if(player2->ctrl->type == CTRL_TYPE_NETWORK) {
                            DEBUG("delaying arena start for %d ticks", player2->ctrl->rtt / 2);
                            scene->gs->next_wait_ticks += player2->ctrl->rtt;
                        }
                    }
                } else {
                    game_state_set_next(scene->gs, SCENE_MECHLAB);
                }
                break;
            case ACT_UP:
            case ACT_LEFT:
                if(game_state_get_player(scene->gs, 1)->selectable) {
                    local->arena--;
                    if(local->arena < 0) {
                        local->arena = 4;
                    }
                    object *arena_select = game_state_find_object(scene->gs, local->arena_select_obj_id);
                    object_select_sprite(arena_select, local->arena);
                }
                break;
            case ACT_DOWN:
            case ACT_RIGHT:
                if(game_state_get_player(scene->gs, 1)->selectable) {
                    local->arena++;
                    if(local->arena > 4) {
                        local->arena = 0;
                    }
                    object *arena_select = game_state_find_object(scene->gs, local->arena_select_obj_id);
                    object_select_sprite(arena_select, local->arena);
                }
                break;
        }
    }
}

void vs_dynamic_tick(scene *scene, int paused) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *i = NULL;
    // Handle extra controller inputs
    i = player1->ctrl->extra_events;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                vs_handle_action(scene, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return;
            }
        } while((i = i->next));
    }
}

void vs_static_tick(scene *scene, int paused) {
    vs_local *local = scene->userdata;
    if(dialog_is_visible(&local->too_pathetic_dialog)) {
        dialog_tick(&local->too_pathetic_dialog);
    } else if(dialog_is_visible(&local->quit_dialog)) {
        dialog_tick(&local->quit_dialog);
    }
}

void vs_input_tick(scene *scene) {
    vs_local *local = scene->userdata;
    ctrl_event *p1 = NULL, *i;
    game_player *player1 = game_state_get_player(scene->gs, 0);
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(i->event_data.action == ACT_ESC) {
                    if(dialog_is_visible(&local->too_pathetic_dialog)) {
                        dialog_event(&local->too_pathetic_dialog, i->event_data.action);
                    } else if(dialog_is_visible(&local->quit_dialog)) {
                        dialog_event(&local->quit_dialog, i->event_data.action);
                    } else if(vs_is_singleplayer(scene) && player1->sp_wins != 0 && !player1->chr) {
                        // there's an active singleplayer campaign, confirm quitting
                        dialog_show(&local->quit_dialog, 1);
                    } else {
                        if(player1->chr) {
                            game_state_set_next(scene->gs, SCENE_MECHLAB);
                        } else {
                            game_state_set_next(scene->gs, SCENE_MELEE);
                        }
                    }
                } else {
                    vs_handle_action(scene, i->event_data.action);
                }
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

void vs_render(scene *scene) {
    vs_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    font_render_shadowed(&font_small, local->vs_str, 160 - ((strlen(local->vs_str) * font_small.w) / 2), 3,
                         COLOR_YELLOW, TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM);

    if(player2->selectable) {
        // arena selection
        video_draw(&local->arena_select_bg, 55, 150);

        // arena name
        font_render_wrapped(&font_small, lang_get(56 + local->arena), 56 + 72, 153, (211 - 72) - 4, TEXT_MEDIUM_GREEN);

        // arena description
        font_render_wrapped(&font_small, lang_get(66 + local->arena), 56 + 72, 160, (211 - 72) - 4, TEXT_MEDIUM_GREEN);
    } else if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK &&
              settings_get()->gameplay.difficulty < 2) {
        // kreissack, but not on Veteran or higher
        font_render_wrapped(&font_small, lang_get(747), 59, 160, 200, COLOR_YELLOW);
    } else if(player1->chr && player2->pilot) {
        font_render_wrapped(&font_small, player2->pilot->quotes[0], 320 - (59 + 150), 165, 120, COLOR_YELLOW);
    } else if(!player2->pilot) {
        // render plug's bitching
        char text[256];
        char money[16];
        fight_stats *fight_stats = &scene->gs->fight_stats;
        snprintf(text, sizeof(text), lang_get(fight_stats->plug_text + PLUG_TEXT_START), fight_stats->sold);
        font_render_wrapped(&font_small, text, 90, 156, 198, COLOR_YELLOW);

        font_render_wrapped(&font_small, "FINANCIAL REPORT", 190, 6, 100, COLOR_GREEN);

        font_render(&font_small, "WINNINGS:", 190, 16, COLOR_DARK_GREEN);
        score_format(fight_stats->winnings, money, sizeof(money));
        snprintf(text, sizeof(text), "$ %sK", money);
        font_render(&font_small, text, 250, 16, COLOR_GREEN);

        font_render(&font_small, "BONUSES:", 196, 24, COLOR_DARK_GREEN);
        score_format(fight_stats->bonuses, money, sizeof(money));
        snprintf(text, sizeof(text), "$ %sK", money);
        font_render(&font_small, text, 250, 24, COLOR_GREEN);

        font_render(&font_small, "REPAIR COST:", 172, 32, COLOR_DARK_GREEN);
        score_format(fight_stats->repair_cost, money, sizeof(money));
        snprintf(text, sizeof(text), "$ %sK", money);
        font_render(&font_small, text, 250, 32, COLOR_GREEN);

        font_render(&font_small, "PROFIT:", 202, 40, COLOR_DARK_GREEN);
        score_format(fight_stats->profit, money, sizeof(money));
        snprintf(text, sizeof(text), "$ %sK", money);
        font_render(&font_small, text, 250, 40, COLOR_GREEN);

        font_render_wrapped(&font_small, "FIGHT STATISTICS", 210, 60, 60, COLOR_GREEN);

        font_render(&font_small, "HITS LANDED:", 202, 79, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u", fight_stats->hits_landed[0]);
        font_render(&font_small, text, 276, 79, COLOR_GREEN);

        font_render(&font_small, "AVERAGE DAMAGE:", 184, 86, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%.1f", fight_stats->average_damage[0]);
        font_render(&font_small, text, 276, 86, COLOR_GREEN);

        font_render(&font_small, "FAILED ATTACKS:", 184, 93, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u", fight_stats->total_attacks[0] - fight_stats->hits_landed[0]);
        font_render(&font_small, text, 276, 93, COLOR_GREEN);

        font_render(&font_small, "HIT/MISS RATIO:", 184, 100, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u%%", fight_stats->hit_miss_ratio[0]);
        font_render(&font_small, text, 276, 100, COLOR_GREEN);

        font_render(&font_small, "OPPONENT", 210, 108, COLOR_GREEN);

        font_render(&font_small, "HITS LANDED:", 202, 115, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u", fight_stats->hits_landed[1]);
        font_render(&font_small, text, 276, 115, COLOR_GREEN);

        font_render(&font_small, "AVERAGE DAMAGE:", 184, 123, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%.1f", fight_stats->average_damage[1]);
        font_render(&font_small, text, 276, 123, COLOR_GREEN);

        font_render(&font_small, "FAILED ATTACKS:", 184, 130, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u", fight_stats->total_attacks[1] - fight_stats->hits_landed[1]);
        font_render(&font_small, text, 276, 130, COLOR_GREEN);

        font_render(&font_small, "HIT/MISS RATIO:", 184, 137, COLOR_DARK_GREEN);
        snprintf(text, sizeof(text), "%u%%", fight_stats->hit_miss_ratio[1]);
        font_render(&font_small, text, 276, 137, COLOR_GREEN);
    } else {
        font_render_wrapped(&font_small, lang_get(749 + (11 * player1->pilot->pilot_id) + player2->pilot->pilot_id), 59,
                            160, 150, COLOR_YELLOW);
        font_render_wrapped(&font_small, lang_get(870 + (11 * player2->pilot->pilot_id) + player1->pilot->pilot_id),
                            320 - (59 + 150), 180, 150, COLOR_YELLOW);
    }
}

void vs_render_overlay(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    if(dialog_is_visible(&local->quit_dialog)) {
        dialog_render(&local->quit_dialog);
    }

    if(dialog_is_visible(&local->too_pathetic_dialog)) {
        dialog_render(&local->too_pathetic_dialog);
    }
}

void vs_quit_dialog_clicked(dialog *dlg, dialog_result result) {
    scene *sc = dlg->userdata;
    if(result == DIALOG_RESULT_YES_OK) {
        game_state_set_next(sc->gs, SCENE_MELEE);
    }
}

void vs_too_pathetic_dialog_clicked(dialog *dlg, dialog_result result) {
    scene *sc = dlg->userdata;
    game_state_set_next(sc->gs, SCENE_MENU);
}

int vs_create(scene *scene) {
    // Init local data
    vs_local *local = omf_calloc(1, sizeof(vs_local));
    scene_set_userdata(scene, local);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    if(player2->pilot == NULL) {
        // display the financial report with your host Plug!

        // generate a new HAR trade list based on your HAR's value
        // and your money
        // TODO figure out how this really works, but this'll do for now
        int trade_value = calculate_trade_value(player1->pilot);
        int8_t trades[10];
        memset(trades, -1, sizeof(trades));
        uint8_t tradecount = 0;

        // collect all the HARs we can afford that are
        // not the current model
        for(int i = 0; i < 11; i++) {
            if(i == player1->pilot->har_id) {
                // don't trade for the current HAR
                continue;
            }
            if(har_price(i) < trade_value + player1->pilot->money) {
                trades[tradecount] = i;
                tradecount++;
            }
        }

        // choose 5 random ones and pack them into the bitmask
        uint16_t new_trades = 0;
        for(int i = 0; i < min2(5, tradecount);) {
            uint8_t choice = rand_int(tradecount);
            if(trades[choice] != -1) {
                new_trades |= 1 << trades[choice];
                trades[choice] = -1;
                i++;
            }
        }

        player1->pilot->har_trades = new_trades;

    } else if(player1->chr) {
        snprintf(local->vs_str, 128, "%s VS. %s", player1->chr->pilot.name, player2->pilot->name);
    } else {
        const char *pilot1 = lang_get(20 + player1->pilot->pilot_id);
        const char *pilot2 = lang_get(20 + player2->pilot->pilot_id);
        snprintf(local->vs_str, 128, "%*.*s VS. %*.*s", (int)strlen(pilot1) - 1, (int)strlen(pilot1) - 1, pilot1,
                 (int)strlen(pilot2) - 1, (int)strlen(pilot2) - 1, pilot2);
    }

    animation *ani;

    palette *mpal = video_get_base_palette();

    palette_load_player_colors(mpal, &player1->pilot->palette, 0);
    if(player2->pilot) {
        palette_load_player_colors(mpal, &player2->pilot->palette, 1);
    }
    video_force_pal_refresh();

    // HAR
    ani = &bk_get_info(scene->bk_data, 5)->ani;
    object *player1_har = omf_calloc(1, sizeof(object));
    object_create(player1_har, scene->gs, vec2i_create(160, 0), vec2f_create(0, 0));
    object_set_animation(player1_har, ani);
    object_select_sprite(player1_har, player1->pilot->har_id);
    object_set_halt(player1_har, 1);
    game_state_add_object(scene->gs, player1_har, RENDER_LAYER_MIDDLE, 0, 0);

    if(player2->pilot) {
        object *player2_har = omf_calloc(1, sizeof(object));
        object_create(player2_har, scene->gs, vec2i_create(160, 0), vec2f_create(0, 0));
        object_set_animation(player2_har, ani);
        object_select_sprite(player2_har, player2->pilot->har_id);
        object_set_direction(player2_har, OBJECT_FACE_LEFT);
        object_set_pal_offset(player2_har, 48);
        object_set_pal_limit(player2_har, 96);
        object_set_halt(player2_har, 1);
        game_state_add_object(scene->gs, player2_har, RENDER_LAYER_MIDDLE, 0, 0);

        // PLAYER
        object *player1_portrait = omf_calloc(1, sizeof(object));
        object_create(player1_portrait, scene->gs, vec2i_create(-10, 150), vec2f_create(0, 0));
        ani = &bk_get_info(scene->bk_data, 4)->ani;
        if(player1->chr) {
            object_set_sprite_override(player1_portrait, 1);
            sprite *sp = omf_calloc(1, sizeof(sprite));
            sprite_create(sp, player1->chr->photo, -1);
            object_set_animation(player1_portrait, create_animation_from_single(sp, vec2i_create(0, 0)));
            object_set_animation_owner(player1_portrait, OWNER_OBJECT);
            player1_portrait->cur_sprite_id = 0;
        } else {
            object_set_animation(player1_portrait, ani);
            object_select_sprite(player1_portrait, player1->pilot->pilot_id);
        }
        object_set_halt(player1_portrait, 1);
        game_state_add_object(scene->gs, player1_portrait, RENDER_LAYER_TOP, 0, 0);

        object *player2_portrait = omf_calloc(1, sizeof(object));
        object_create(player2_portrait, scene->gs, vec2i_create(330, 150), vec2f_create(0, 0));
        if(player1->chr) {
            object_set_sprite_override(player2_portrait, 1);
            sprite *sp = omf_calloc(1, sizeof(sprite));
            sprite_create(sp, player2->pilot->photo, -1);
            object_set_animation(player2_portrait, create_animation_from_single(sp, vec2i_create(0, 0)));
            object_set_animation_owner(player2_portrait, OWNER_OBJECT);
            player2_portrait->cur_sprite_id = 0;
        } else {
            object_set_animation(player2_portrait, ani);
            object_select_sprite(player2_portrait, player2->pilot->pilot_id);
        }
        object_set_direction(player2_portrait, OBJECT_FACE_LEFT);
        object_set_halt(player2_portrait, 1);
        game_state_add_object(scene->gs, player2_portrait, RENDER_LAYER_TOP, 0, 0);

    } else {
        // plug time!!!!!!!111eleven!
        object *plug = omf_calloc(1, sizeof(object));
        object_create(plug, scene->gs, vec2i_create(-10, 150), vec2f_create(0, 0));
        ani = &bk_get_info(scene->bk_data, 2)->ani;
        object_set_animation(plug, ani);
        object_select_sprite(plug, 0);
        object_set_halt(plug, 1);
        game_state_add_object(scene->gs, plug, RENDER_LAYER_TOP, 0, 0);
    }

    if(player2->pilot != NULL) {
        // clone the left side of the background image
        // Note! We are touching the scene-wide background surface!
        surface_sub(&scene->bk_data->background, // DST Surface
                    &scene->bk_data->background, // SRC Surface
                    160, 0,                      // DST
                    0, 0,                        // SRC
                    160, 200,                    // Size
                    SUB_METHOD_MIRROR);          // Flip the right side horizontally
    }

    if(player2->selectable) {
        // player1 gets to choose, start at arena 0
        local->arena = 0;
    } else if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK) {
        // force arena 0 when fighting Kreissack in 1 player mode
        local->arena = 0;
    } else {
        // pick a random arena for 1 player mode
        local->arena = rand_int(5); // srand was done in melee
    }

    // Arena
    if(player2->selectable) {
        ani = &bk_get_info(scene->bk_data, 3)->ani;
        object *arena_select = omf_calloc(1, sizeof(object));
        object_create(arena_select, scene->gs, vec2i_create(59, 155), vec2f_create(0, 0));
        local->arena_select_obj_id = arena_select->id;
        object_set_animation(arena_select, ani);
        object_select_sprite(arena_select, local->arena);
        object_set_halt(arena_select, 1);
        game_state_add_object(scene->gs, arena_select, RENDER_LAYER_TOP, 0, 0);
    }

    // SCIENTIST
    int scientistpos = rand_int(4);
    if(!player2->pilot && scientistpos % 2 == 1) {
        // there is no right hand gantry
        // so if the position is odd, sub 1
        // to force it to the left side
        scientistpos -= 1;
    }
    vec2i scientistcoord = spawn_position(scientistpos, 1);
    if(scientistpos % 2) {
        scientistcoord.x += 50;
    } else {
        scientistcoord.x -= 50;
    }
    object *o_scientist = omf_calloc(1, sizeof(object));
    ani = &bk_get_info(scene->bk_data, 8)->ani;
    object_create(o_scientist, scene->gs, scientistcoord, vec2f_create(0, 0));
    object_set_animation(o_scientist, ani);
    object_select_sprite(o_scientist, 0);
    object_set_direction(o_scientist, scientistpos % 2 ? OBJECT_FACE_LEFT : OBJECT_FACE_RIGHT);
    game_state_add_object(scene->gs, o_scientist, RENDER_LAYER_MIDDLE, 0, 0);

    // WELDER
    int welderpos = rand_int(6);
    // On non-tournament mode, the welder cannot be on the same gantry or the
    // same *side* as the scientist he also can't be on the same 'level' but he
    // has 10 possible starting positions
    if(player2->pilot) {
        while((welderpos % 2) == (scientistpos % 2) || (scientistpos < 2 && welderpos < 2) ||
              (scientistpos > 1 && welderpos > 1 && welderpos < 4)) {
            welderpos = rand_int(6);
        }
    } else {
        // On tournament mode, the welder cannot be on the same level as the
        // scientist.
        while((welderpos % 2) == 1 || (scientistpos == welderpos)) {
            welderpos = rand_int(3) * 2;
        }
    }
    object *o_welder = omf_calloc(1, sizeof(object));
    ani = &bk_get_info(scene->bk_data, 7)->ani;
    object_create(o_welder, scene->gs, spawn_position(welderpos, 0), vec2f_create(0, 0));
    object_set_animation(o_welder, ani);
    object_select_sprite(o_welder, 0);
    object_set_spawn_cb(o_welder, cb_vs_spawn_object, (void *)scene);
    object_set_destroy_cb(o_welder, cb_vs_destroy_object, (void *)scene);
    object_set_direction(o_welder, welderpos % 2 ? OBJECT_FACE_LEFT : OBJECT_FACE_RIGHT);
    game_state_add_object(scene->gs, o_welder, RENDER_LAYER_MIDDLE, 0, 0);

    // GANTRIES
    object *o_gantry_a = omf_calloc(1, sizeof(object));
    ani = &bk_get_info(scene->bk_data, 11)->ani;
    object_create(o_gantry_a, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
    object_set_animation(o_gantry_a, ani);
    object_select_sprite(o_gantry_a, 0);
    game_state_add_object(scene->gs, o_gantry_a, RENDER_LAYER_TOP, 0, 0);

    if(player2->pilot) {
        object *o_gantry_b = omf_calloc(1, sizeof(object));
        object_create(o_gantry_b, scene->gs, vec2i_create(320, 0), vec2f_create(0, 0));
        object_set_animation(o_gantry_b, ani);
        object_select_sprite(o_gantry_b, 0);
        object_set_direction(o_gantry_b, OBJECT_FACE_LEFT);
        game_state_add_object(scene->gs, o_gantry_b, RENDER_LAYER_TOP, 0, 0);
    }

    // Background tex
    menu_background2_create(&local->arena_select_bg, 211, 50);

    // Quit Dialog
    dialog_create(&local->quit_dialog, DIALOG_STYLE_YES_NO, "ARE YOU SURE YOU WANT TO QUIT THIS GAME?", 72, 60);
    local->quit_dialog.userdata = scene;
    local->quit_dialog.clicked = vs_quit_dialog_clicked;

    // Too Pathetic Dialog
    char insult[512];
    snprintf(insult, 512, lang_get(748), "Veteran", "Major Kreissack");
    dialog_create(&local->too_pathetic_dialog, DIALOG_STYLE_OK, insult, 72, 60);
    local->too_pathetic_dialog.userdata = scene;
    local->too_pathetic_dialog.clicked = vs_too_pathetic_dialog_clicked;

    if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK && settings_get()->gameplay.difficulty < 2) {
        // kreissack, but not on Veteran or higher
        dialog_show(&local->too_pathetic_dialog, 1);
    }

    // Callbacks
    scene_set_render_cb(scene, vs_render);
    scene_set_render_overlay_cb(scene, vs_render_overlay);
    scene_set_input_poll_cb(scene, vs_input_tick);
    scene_set_dynamic_tick_cb(scene, vs_dynamic_tick);
    scene_set_static_tick_cb(scene, vs_static_tick);
    scene_set_free_cb(scene, vs_free);

    return 0;
}
