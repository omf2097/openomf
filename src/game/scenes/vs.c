#include "game/scenes/vs.h"
#include "controller/controller.h"
#include "game/game_state.h"
#include "game/gui/dialog.h"
#include "game/gui/menu_background.h"
#include "game/gui/text/text.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/utils/formatting.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "video/video.h"
#include <stdio.h>
#include <stdlib.h>

#define TEXT_GREEN 0xA7
#define TEXT_SHADOW_GREEN 0xA2

void cb_vs_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata);
void cb_vs_destroy_object(object *parent, int id, void *userdata);

typedef struct report_card {
    text *plug_whine;
    text *report_title;
    text *report_left;
    text *report_right;
    text *stats_title;
    text *stats_self_left;
    text *stats_self_right;
    text *opponent_title;
    text *opponent_right;
} report_card;

typedef struct vs_local {
    int arena_select_obj_id;
    surface arena_select_bg;
    dialog quit_dialog;
    dialog too_pathetic_dialog;

    text *vs_text;
    text *insults[2];
    text *arena_name;
    text *arena_desc;

    report_card *report;
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
// Welder does an additional roll for the 3 places on the torso.
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

void cb_vs_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t mp_flags, int s, int g, void *userdata) {
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

static void free_report_card(report_card *card) {
    if(card == NULL)
        return;
    text_free(&card->plug_whine);
    text_free(&card->report_title);
    text_free(&card->report_left);
    text_free(&card->report_right);
    text_free(&card->stats_title);
    text_free(&card->stats_self_left);
    text_free(&card->stats_self_right);
    text_free(&card->opponent_title);
    text_free(&card->opponent_right);
    omf_free(card);
}

static void vs_free(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    text_free(&local->vs_text);
    text_free(&local->arena_name);
    text_free(&local->arena_desc);
    text_free(&local->insults[0]);
    text_free(&local->insults[1]);
    free_report_card(local->report);
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
                    game_state_set_next(scene->gs, SCENE_ARENA0 + scene->gs->arena);
                } else {
                    game_state_get_player(scene->gs, 1)->pilot = NULL;
                    if(scene->gs->fight_stats.challenger) {
                        // unranked challenger time
                        game_state_set_next(scene->gs, SCENE_NEWSROOM);
                    } else {
                        game_state_set_next(scene->gs, SCENE_MECHLAB);
                    }
                }
                break;
            case ACT_UP:
            case ACT_LEFT:
                if(game_state_get_player(scene->gs, 1)->selectable) {
                    scene->gs->arena--;
                    if(scene->gs->arena < 0) {
                        scene->gs->arena = 4;
                    }
                    object *arena_select = game_state_find_object(scene->gs, local->arena_select_obj_id);
                    object_select_sprite(arena_select, scene->gs->arena);

                    text_set_from_c(local->arena_name, lang_get(56 + scene->gs->arena));
                    text_set_from_c(local->arena_desc, lang_get(66 + scene->gs->arena));
                }
                break;
            case ACT_DOWN:
            case ACT_RIGHT:
                if(game_state_get_player(scene->gs, 1)->selectable) {
                    scene->gs->arena++;
                    if(scene->gs->arena > 4) {
                        scene->gs->arena = 0;
                    }
                    object *arena_select = game_state_find_object(scene->gs, local->arena_select_obj_id);
                    object_select_sprite(arena_select, scene->gs->arena);

                    text_set_from_c(local->arena_name, lang_get(56 + scene->gs->arena));
                    text_set_from_c(local->arena_desc, lang_get(66 + scene->gs->arena));
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
    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *menu_ev = NULL;
    game_state_menu_poll(scene->gs, &menu_ev);

    for(ctrl_event *i = menu_ev; i; i = i->next) {
        if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_ESC) {
            if(dialog_is_visible(&local->too_pathetic_dialog)) {
                dialog_event(&local->too_pathetic_dialog, i->event_data.action);
            } else if(dialog_is_visible(&local->quit_dialog)) {
                dialog_event(&local->quit_dialog, i->event_data.action);
            } else if(vs_is_singleplayer(scene) && player1->sp_wins != 0 && !player1->chr) {
                // there's an active singleplayer campaign, confirm quitting
                dialog_show(&local->quit_dialog, 1);
            } else {
                if(player1->chr) {
                    // Match cancelled, no winner
                    scene->gs->fight_stats.winner = -1;
                    // null out the  p2 pilot
                    game_state_get_player(scene->gs, 1)->pilot = NULL;
                    game_state_set_next(scene->gs, SCENE_MECHLAB);
                } else if(is_spectator(scene->gs)) {
                    game_state_set_next(scene->gs, SCENE_LOBBY);
                } else {
                    game_state_set_next(scene->gs, SCENE_MELEE);
                }
            }
        }
    }
    controller_free_chain(menu_ev);

    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                vs_handle_action(scene, i->event_data.action);
            } else if(i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
            }
        } while((i = i->next));
    }
    controller_free_chain(p1);
}

/**
 * Create text entry for the end-of-tournament stats title texts
 */
static text *create_title_text(const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, 150, 30);
    text_set_from_c(t, str);
    text_set_color(t, TEXT_GREEN);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_generate_layout(t);
    return t;
}

/**
 * Create text entry for the end-of-tournament stats label texts
 */
static text *create_labels_text(const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, 150, 30);
    text_set_from_c(t, str);
    text_set_color(t, TEXT_DARK_GREEN);
    text_set_line_spacing(t, 1);
    text_set_horizontal_align(t, TEXT_ALIGN_RIGHT);
    text_generate_layout(t);
    return t;
}

/**
 * Create text entry for the end-of-tournament stats value texts (money, etc.)
 */
static text *create_values_text(const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, 150, 30);
    text_set_from_c(t, str);
    text_set_color(t, TEXT_GREEN);
    text_set_line_spacing(t, 1);
    text_set_horizontal_align(t, TEXT_ALIGN_LEFT);
    text_generate_layout(t);
    return t;
}

/**
 * Create statistics texts for tournament plug screen. This should only be called if needed to save memory.
 */
static report_card *create_report_card(const fight_stats *fight_stats) {
    char text[256];
    char money[4][16];
    report_card *card = omf_calloc(1, sizeof(report_card));

    card->plug_whine = text_create_with_font_and_size(FONT_SMALL, 200, 55);
    text_set_color(card->plug_whine, COLOR_YELLOW);
    text_set_shadow_style(card->plug_whine, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    text_set_shadow_color(card->plug_whine, 202);
    text_set_horizontal_align(card->plug_whine, TEXT_ALIGN_CENTER);
    snprintf(text, sizeof(text), lang_get(fight_stats->plug_text + PLUG_TEXT_START), fight_stats->sold);
    text_set_from_c(card->plug_whine, text);

    // These are all static. Note that for opponent labels, we reuse the own stats label.
    card->report_title = create_title_text("FINANCIAL REPORT");
    card->stats_title = create_title_text("FIGHT\nSTATISTICS");
    card->opponent_title = create_title_text("OPPONENT");
    card->report_left = create_labels_text("WINNINGS:\nBONUSES:\nREPAIR COST:\nPROFIT:");
    card->stats_self_left = create_labels_text("HITS LANDED:\nAVERAGE DAMAGE:\nFAILED ATTACKS:\nHIT/MISS RATIO:");

    score_format(fight_stats->winnings, money[0], sizeof(money[0]));
    score_format(fight_stats->bonuses, money[1], sizeof(money[1]));
    score_format(fight_stats->repair_cost, money[2], sizeof(money[2]));
    score_format(fight_stats->profit, money[3], sizeof(money[3]));
    snprintf(text, sizeof(text), "$ %sK\n$ %sK\n$ %sK\n$ %sK", money[0], money[1], money[2], money[3]);
    card->report_right = create_values_text(text);

    snprintf(text, sizeof(text), "%u\n%.1f\n%u\n%u%%", fight_stats->hits_landed[0], fight_stats->average_damage[0],
             fight_stats->total_attacks[0] - fight_stats->hits_landed[0], fight_stats->hit_miss_ratio[0]);
    card->stats_self_right = create_values_text(text);

    snprintf(text, sizeof(text), "%u\n%.1f\n%u\n%u%%", fight_stats->hits_landed[1], fight_stats->average_damage[1],
             fight_stats->total_attacks[1] - fight_stats->hits_landed[1], fight_stats->hit_miss_ratio[1]);
    card->opponent_right = create_values_text(text);

    return card;
}

/**
 * Only called if the scene is the Plug end-of-match screen.
 */
static void vs_render_fight_stats(scene *scene) {
    vs_local *local = scene->userdata;
    report_card *card = local->report;
    assert(card != NULL);

    text_draw(card->plug_whine, 90, 156);

    text_draw(card->report_title, 163, 6);
    text_draw(card->report_left, 250 - 150 - 6, 16);
    text_draw(card->report_right, 250, 16);

    text_draw(card->stats_title, 163, 60);
    text_draw(card->stats_self_left, 276 - 150 - 6, 79);
    text_draw(card->stats_self_right, 276, 79);

    text_draw(card->opponent_title, 163, 108);
    text_draw(card->stats_self_left, 276 - 150 - 6, 115);
    text_draw(card->opponent_right, 276, 115);
}

/**
 * This creates the text object for the top X VS. Y title.
 */
static text *create_vs_text(const char *str) {
    text *t = text_create_with_font_and_size(FONT_SMALL, 320, 6);
    text_set_from_c(t, str);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_set_color(t, COLOR_YELLOW);
    text_set_shadow_color(t, 202);
    text_set_shadow_style(t, GLYPH_SHADOW_RIGHT | GLYPH_SHADOW_BOTTOM);
    return t;
}

/**
 * This creates the text object for the arena name and description in 2 player game
 */
static text *create_arena_text(const char *str, int w, int h) {
    text *t = text_create_with_font_and_size(FONT_SMALL, w, h);
    text_set_from_c(t, str);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_set_vertical_align(t, TEXT_ALIGN_MIDDLE);
    text_set_color(t, COLOR_GREEN);
    return t;
}

static text *create_insult_text(const char *str, int w, int h) {
    text *t = text_create_with_font_and_size(FONT_SMALL, w, h);
    text_set_from_c(t, str);
    text_set_horizontal_align(t, TEXT_ALIGN_CENTER);
    text_set_vertical_align(t, TEXT_ALIGN_MIDDLE);
    text_set_color(t, COLOR_YELLOW);
    return t;
}

static void vs_render(scene *scene) {
    vs_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    if(player2->pilot) {
        text_draw(local->vs_text, 0, 3);
    }

    if(player2->selectable) {
        // arena selection
        video_draw(&local->arena_select_bg, 55, 150);
        text_draw(local->arena_name, 56 + 72, 152);
        text_draw(local->arena_desc, 56 + 72, 153);
    } else if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK &&
              settings_get()->gameplay.difficulty < 2) {
        // kreissack, but not on Veteran or higher
        text_draw(local->insults[1], 80, 165);
    } else if(player1->chr && player2->pilot) {
        // tournament insults
        text_draw(local->insults[1], 100, 145);
    } else if(player2->pilot == NULL) {
        // plug screen fight stats
        if(scene->gs->fight_stats.winner >= 0) {
            vs_render_fight_stats(scene);
        }
    } else {
        // 1 player mode insults
        text_draw(local->insults[0], 77, 150);
        text_draw(local->insults[1], 110, 170);
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
    } else {
        dialog_show(dlg, 0);
    }
}

void vs_too_pathetic_dialog_clicked(dialog *dlg, dialog_result result) {
    scene *sc = dlg->userdata;
    game_state_set_next(sc->gs, SCENE_SCOREBOARD);
}

int vs_create(scene *scene) {
    // Initialize Demo
    if(is_demoplay(scene->gs)) {
        game_state_init_demo(scene->gs);
    }

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
    } else {
        char title[128];
        snprintf(title, 128, "%s VS. %s", player1->pilot->name, player2->pilot->name);
        local->vs_text = create_vs_text(title);
    }

    // Set player palettes
    palette_load_player_colors(&player1->pilot->palette, 0);
    if(player2->pilot) {
        palette_load_player_colors(&player2->pilot->palette, 1);
    }

    // HAR
    animation *ani;
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
        fight_stats *fight_stats = &scene->gs->fight_stats;
        object_create(plug, scene->gs, vec2i_create(-10, 150), vec2f_create(0, 0));
        ani = &bk_get_info(scene->bk_data, 2)->ani;
        object_set_animation(plug, ani);
        // plug should be happy, sometimes? he is happy on frame 1
        if(fight_stats->plug_text == PLUG_ENHANCEMENT || fight_stats->plug_text == PLUG_WIN_BIG) {
            object_select_sprite(plug, 1);
        } else {
            object_select_sprite(plug, 0);
        }
        object_set_halt(plug, 1);
        game_state_add_object(scene->gs, plug, RENDER_LAYER_TOP, 0, 0);
        local->report = create_report_card(fight_stats); // Set up the statistics texts to the right side of the scene

        // Let's add some scrapes
        sprite *har_sprite = animation_get_sprite(player1_har->cur_animation, player1->pilot->har_id);
        ani = &bk_get_info(scene->bk_data, 9)->ani;

        int n_scrapes = ((fight_stats->max_hp - fight_stats->hp) * 170) / fight_stats->max_hp;
        log_debug("Creating %d scrapes", n_scrapes);
        int scrape_no;
        for(int n = 0; n < n_scrapes; n++) {
            scrape_no = rand_int(animation_get_sprite_count(ani));
            surface_multiply_decal(har_sprite->data, animation_get_sprite(ani, scrape_no)->data, rand_int(160),
                                   rand_int(120));
        }
        player1_har->cur_surface = har_sprite->data;
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

    if(player2->selectable && !is_spectator(scene->gs)) {
        // player1 gets to choose, start at arena 0
        scene->gs->arena = 0;
        local->arena_name = create_arena_text(lang_get(56 + scene->gs->arena), (211 - 74), 6);
        local->arena_desc = create_arena_text(lang_get(66 + scene->gs->arena), (211 - 74), 50);
    } else if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK) {
        // force arena 0 when fighting Kreissack in 1 player mode
        scene->gs->arena = 0;
    } else if(is_tournament(scene->gs) || is_demoplay(scene->gs)) {
        // pick random arenas
        scene->gs->arena = rand_int(5);
    } else if(is_spectator(scene->gs)) {
        local->arena_name = create_arena_text(lang_get(56 + scene->gs->arena), (211 - 74), 6);
        local->arena_desc = create_arena_text(lang_get(66 + scene->gs->arena), (211 - 74), 50);
    } else {
        // 1 player mode cycles through the arenas
    }

    // Insults
    if(player2->pilot && player2->pilot->pilot_id == PILOT_KREISSACK && settings_get()->gameplay.difficulty < 2) {
        // kreissack, but not on Veteran or higher
        local->insults[0] = NULL;
        local->insults[1] = create_insult_text(lang_get(747), 170, 60);
    } else if(player1->chr && player2->pilot) {
        // tournament mode
        local->insults[0] = NULL;
        local->insults[1] = create_insult_text(player2->pilot->quotes[0], 150, 60);
    } else if(player2->pilot) {
        // 1 player
        local->insults[0] =
            create_insult_text(lang_get(749 + (11 * player1->pilot->pilot_id) + player2->pilot->pilot_id), 150, 30);
        local->insults[1] =
            create_insult_text(lang_get(870 + (11 * player2->pilot->pilot_id) + player1->pilot->pilot_id), 150, 30);
    }

    // Arena
    if(player2->selectable) {
        ani = &bk_get_info(scene->bk_data, 3)->ani;
        object *arena_select = omf_calloc(1, sizeof(object));
        object_create(arena_select, scene->gs, vec2i_create(59, 155), vec2f_create(0, 0));
        local->arena_select_obj_id = arena_select->id;
        object_set_animation(arena_select, ani);
        object_select_sprite(arena_select, scene->gs->arena);
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
    menu_background_create(&local->arena_select_bg, 211, 50, MenuBackgroundMeleeVs);

    // Quit Dialog
    dialog_create(&local->quit_dialog, DIALOG_STYLE_YES_NO, "Are you sure you want to quit this game?", 72, 60);
    local->quit_dialog.userdata = scene;
    local->quit_dialog.clicked = vs_quit_dialog_clicked;

    // Too Pathetic Dialog

    str insult;
    str_from_c(&insult, lang_get(748));
    str_replace(&insult, "%s", lang_get(345), 1);
    str_replace(&insult, "%s", lang_get(30), 1);
    // XXX HACK: Remove newline after kreissack's name until we clean up our string tables
    str_replace(&insult, "\n.", ".", -1);
    dialog_create(&local->too_pathetic_dialog, DIALOG_STYLE_OK, str_c(&insult), 40, 40);
    str_free(&insult);
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
    scene_set_dynamic_tick_cb(scene, vs_dynamic_tick); // used for netplay
    scene_set_static_tick_cb(scene, vs_static_tick);
    scene_set_free_cb(scene, vs_free);

    return 0;
}
