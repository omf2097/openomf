#include <SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "audio/audio.h"
#include "controller/controller.h"
#include "controller/net_controller.h"
#include "formats/error.h"
#include "formats/rec.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/gui/button.h"
#include "game/gui/filler.h"
#include "game/gui/gui_frame.h"
#include "game/gui/label.h"
#include "game/gui/menu.h"
#include "game/gui/progressbar.h"
#include "game/gui/text_render.h"
#include "game/gui/textslider.h"
#include "game/gui/widget.h"
#include "game/objects/arena_constraints.h"
#include "game/objects/har.h"
#include "game/objects/hazard.h"
#include "game/objects/scrap.h"
#include "game/protos/object.h"
#include "game/scenes/arena.h"
#include "game/scenes/mechlab/lab_menu_customize.h"
#include "game/utils/score.h"
#include "game/utils/settings.h"
#include "game/utils/ticktimer.h"
#include "resources/languages.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/random.h"
#include "video/video.h"

// the pilot name, HAR, and score
#define TEXT_HUD_COLOR 0xE7
#define TEXT_HUD_SHADOW 0xF7

#define HAR1_START_POS 110
#define HAR2_START_POS 210

#define GAME_MENU_RETURN_ID 100
#define GAME_MENU_QUIT_ID 101

// Colors specific to palette used by arena
#define TEXT_PRIMARY_COLOR 0xFE
#define TEXT_SECONDARY_COLOR 0xFD
#define TEXT_DISABLED_COLOR 0xC0
#define TEXT_ACTIVE_COLOR 0xFF
#define TEXT_INACTIVE_COLOR 0xFE
#define TEXT_SHADOW_COLOR 0xC0

typedef enum
{
    NONE = 0,
    YOULOSE,
    YOUWIN,
    DONE
} win_state;

typedef struct arena_local_t {
    gui_frame *game_menu;

    int menu_visible;
    unsigned int state;
    int ending_ticks;

    component *health_bars[2];
    component *endurance_bars[2];

    int round;
    int rounds;
    int over;
    int winner;
    bool tournament;
    win_state win_state;

    int player_rounds[2][4];

    int rein_enabled;

    sd_action rec_last[2];
} arena_local;

void write_rec_move(scene *scene, game_player *player, int action);

bool defeated_at_rest(object *obj);

static void arena_end(scene *sc);

// -------- Local callbacks --------

void game_menu_quit(component *c, void *userdata) {
    scene *s = userdata;
    arena_local *local = scene_get_userdata((scene *)userdata);
    local->winner = 1;

    s->gs->fight_stats.plug_text = PLUG_FORFEIT;
    chr_score_reset(game_player_get_score(game_state_get_player((s)->gs, 0)), 1);
    chr_score_reset(game_player_get_score(game_state_get_player((s)->gs, 1)), 1);

    game_player *player1 = game_state_get_player(((scene *)userdata)->gs, 0);
    if(s->gs->init_flags->playback == 1) {
        // 'quit' button exits during REC playback
        game_state_set_next(s->gs, SCENE_NONE);
    } else if(player1->chr) {
        // quit back to VS for plug to call you a chicken
        if(s->gs->match_settings.sim) {
            game_state_set_next(s->gs, SCENE_MECHLAB);
        } else {
            game_state_set_next(s->gs, SCENE_VS);
        }
    } else {
        game_state_set_next(s->gs, SCENE_MENU);
    }

    arena_end(s);

    if(player1->chr) {
        game_player *player2 = game_state_get_player(((scene *)userdata)->gs, 1);
        player2->pilot = NULL;
    }
}

void game_menu_return(component *c, void *userdata) {
    arena_local *local = scene_get_userdata((scene *)userdata);
    local->menu_visible = 0;
    game_state_set_paused(((scene *)userdata)->gs, 0);
}

void arena_music_slide(component *c, void *userdata, int pos) {
    audio_set_music_volume(pos / 10.0f);
}

void arena_sound_slide(component *c, void *userdata, int pos) {
    audio_set_sound_volume(pos / 10.0f);
}

void arena_speed_slide(component *c, void *userdata, int pos) {
    scene *sc = userdata;
    game_state_set_speed(sc->gs, pos + 5);
}

void scene_fight_anim_done(void *scenedata, void *userdata) {
    scene *scene = scenedata;
    // int parent_id = userdata;
    // object *parent = game_state_find_object(scene->gs, parent_id;
    arena_local *arena = scene_get_userdata(scene);

    // This will release HARs for action
    arena->state = ARENA_STATE_FIGHTING;

    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    // parent->animation_state.finished = 1;
}

void scene_fight_anim_start(void *scenedata, void *userdata) {
    // Start FIGHT animation
    scene *sc = scenedata;
    game_state *gs = sc->gs;
    scene *scene = game_state_get_scene(gs);
    animation *fight_ani = &bk_get_info(scene->bk_data, 10)->ani;
    object *fight = omf_calloc(1, sizeof(object));
    object_create(fight, gs, fight_ani->start_pos, vec2f_create(0, 0));
    object_set_stl(fight, bk_get_stl(scene->bk_data));
    object_set_animation(fight, fight_ani);
    // object_set_finish_cb(fight, scene_fight_anim_done);
    game_state_add_object(gs, fight, RENDER_LAYER_TOP, 0, 0);
    ticktimer_add(&scene->tick_timer, 24, scene_fight_anim_done, NULL /*fight->id*/);
}

void scene_ready_anim_done(object *parent) {
    // Wait a moment before loading FIGHT animation
    ticktimer_add(&game_state_get_scene(parent->gs)->tick_timer, 10, scene_fight_anim_start, NULL);

    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
}

void scene_youwin_anim_done(object *parent) {
    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
    arena_local *local = scene_get_userdata(parent->gs->sc);
    local->win_state = DONE;
}

void scene_youwin_anim_start(void *userdata) {
    // Start FIGHT animation
    game_state *gs = userdata;
    scene *scene = game_state_get_scene(gs);
    animation *youwin_ani = &bk_get_info(scene->bk_data, 9)->ani;
    object *youwin = omf_calloc(1, sizeof(object));
    object_create(youwin, gs, youwin_ani->start_pos, vec2f_create(0, 0));
    object_set_stl(youwin, bk_get_stl(scene->bk_data));
    object_set_animation(youwin, youwin_ani);
    object_set_finish_cb(youwin, scene_youwin_anim_done);
    game_state_add_object(gs, youwin, RENDER_LAYER_MIDDLE, 0, 0);

    // This will release HARs for action
    /*arena->state = ARENA_STATE_ENDING;*/
}

void scene_youlose_anim_done(object *parent) {
    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
    arena_local *local = scene_get_userdata(parent->gs->sc);
    local->win_state = DONE;
}

void scene_youlose_anim_start(void *userdata) {
    // Start FIGHT animation
    game_state *gs = userdata;
    scene *scene = game_state_get_scene(gs);
    animation *youlose_ani = &bk_get_info(scene->bk_data, 8)->ani;
    object *youlose = omf_calloc(1, sizeof(object));
    object_create(youlose, gs, youlose_ani->start_pos, vec2f_create(0, 0));
    object_set_stl(youlose, bk_get_stl(scene->bk_data));
    object_set_animation(youlose, youlose_ani);
    object_set_finish_cb(youlose, scene_youlose_anim_done);
    game_state_add_object(gs, youlose, RENDER_LAYER_MIDDLE, 0, 0);

    // This will release HARs for action
    /*arena->state = ARENA_STATE_ENDING;*/
}

void arena_screengrab_winner(scene *sc) {
    game_state *gs = sc->gs;

    // take victory pose screenshot for the newsroom
    object *o1 = game_state_find_object(gs, game_state_get_player(gs, 0)->har_obj_id);
    har *h1 = object_get_userdata(o1);
    if(h1->state == STATE_VICTORY || h1->state == STATE_DONE) {
        har_screencaps_capture(&game_state_get_player(gs, 0)->screencaps, o1, NULL, SCREENCAP_POSE);
    } else {
        object *o2 = game_state_find_object(gs, game_state_get_player(gs, 0)->har_obj_id);
        har_screencaps_capture(&game_state_get_player(gs, 1)->screencaps, o2, NULL, SCREENCAP_POSE);
    }
}

static void arena_end(scene *sc) {
    game_state *gs = sc->gs;
    arena_local *local = scene_get_userdata(sc);
    const scene *scene = game_state_get_scene(gs);
    fight_stats *fight_stats = &gs->fight_stats;

    int winner_player_id = local->winner;
    int loser_player_id = !winner_player_id;
    game_player *player_winner = game_state_get_player(scene->gs, winner_player_id);
    game_player *player_loser = game_state_get_player(scene->gs, loser_player_id);
    object *winner = game_state_find_object(scene->gs, game_player_get_har_obj_id(player_winner));
    har *winner_har = object_get_userdata(winner);

    // if tournament player won
    if(is_tournament(gs) && local->winner == 0) {
        // TODO The repair costs formula here is completely bogus
        int trade_value = calculate_trade_value(player_winner->pilot) / 100;
        float hp_percentage = (float)winner_har->health / (float)winner_har->health_max;
        fight_stats->repair_cost = (1.0f - hp_percentage) * trade_value;

        float winnings_multiplier = player_winner->chr->winnings_multiplier;
        fight_stats->winnings = (player_loser->pilot->money + player_loser->pilot->winnings) * winnings_multiplier;
        fight_stats->winnings += (int)(400 * hp_percentage);

        // secret players have no rank, and don't increase your own ranking
        if(!gs->match_settings.sim && player_loser->pilot->rank > 0) {
            player_winner->pilot->rank--;
            if(player_winner->pilot->rank < 1) {
                player_winner->pilot->rank = 1;
            }
            for(int i = 0; i < player_winner->chr->pilot.enemies_inc_unranked; i++) {
                if(player_winner->chr->enemies[i]->pilot.rank == player_winner->pilot->rank) {
                    player_winner->chr->enemies[i]->pilot.rank += 1;
                    break;
                }
            }
        }
    }
    // if tournament player lost
    else if(is_tournament(gs) && local->winner == 1) {
        // secret players have no rank, and don't decrease your own ranking
        if(player_loser->pilot->rank <= player_loser->pilot->enemies_ex_unranked && !gs->match_settings.sim &&
           player_winner->pilot->rank > 0) {
            player_loser->pilot->rank++;
            if(player_loser->pilot->rank > player_loser->chr->pilot.enemies_ex_unranked + 1) {
                player_loser->pilot->rank = player_loser->chr->pilot.enemies_ex_unranked + 1;
            }
            for(int i = 0; i < player_loser->chr->pilot.enemies_inc_unranked; i++) {
                if(player_loser->chr->enemies[i]->pilot.rank == player_loser->pilot->rank) {
                    player_loser->chr->enemies[i]->pilot.rank -= 1;
                    break;
                }
            }
        }
        fight_stats->repair_cost = calculate_trade_value(player_loser->pilot) / 100;
    }

    if(!gs->match_settings.sim) {
        game_player_get_score(player_winner)->wins++;
        player_winner->pilot->wins++;
        player_loser->pilot->losses++;
    }

    // Switch scene
    if(scene->gs->init_flags->playback == 1) {
        // exit after REC playback
        game_state_set_next(scene->gs, SCENE_NONE);
    } else if(is_singleplayer(gs) || is_tournament(gs) || is_demoplay(gs)) {
        game_player *p1 = game_state_get_player(gs, 0);
        game_player *p2 = game_state_get_player(gs, 1);
        har *p1_har = object_get_userdata(game_state_find_object(gs, game_player_get_har_obj_id(p1)));
        har *p2_har = object_get_userdata(game_state_find_object(gs, game_player_get_har_obj_id(p2)));

        // Convert screen captures to grayscale
        har_screencaps *caps = &(fight_stats->winner == 0 ? p1 : p2)->screencaps;
        vga_palette *pal = bk_get_palette(scene->bk_data, 0);
        har_screencaps_compress(caps, pal, SCREENCAP_BLOW);
        har_screencaps_compress(caps, pal, SCREENCAP_POSE);

        // Set the fight statistics and Plug McEllis's complaints
        fight_stats->bonuses = game_player_get_score(p1)->score / 1000;
        fight_stats->profit = fight_stats->bonuses + fight_stats->winnings - fight_stats->repair_cost;
        bool warning_given = p1->pilot->money < 0;

        if(!gs->match_settings.sim) {
            p1->pilot->money += fight_stats->profit;
        }
        if(fight_stats->hits_landed[0] != 0) {
            fight_stats->average_damage[0] =
                (float)(p2_har->health_max - p2_har->health) / (float)fight_stats->hits_landed[0];
        }
        if(fight_stats->total_attacks[0] != 0) {
            fight_stats->hit_miss_ratio[0] = 100 * fight_stats->hits_landed[0] / fight_stats->total_attacks[0];
        }
        if(fight_stats->hits_landed[1] != 0) {
            fight_stats->average_damage[1] =
                (float)(p1_har->health_max - p1_har->health) / (float)fight_stats->hits_landed[1];
        }
        if(fight_stats->total_attacks[1] != 0) {
            fight_stats->hit_miss_ratio[1] = 100 * fight_stats->hits_landed[1] / fight_stats->total_attacks[1];
        }
        if(fight_stats->winner == 0) {
            int16_t hp_left_percent = har_health_percent(p1_har);
            // check if this is an unranked challenger with an enhancement we don't have
            if(p2->pilot->rank == 0 && fight_stats->finish == FINISH_DESTRUCTION &&
               p2->pilot->enhancements[p1->pilot->har_id] == p1->pilot->enhancements[p1->pilot->har_id] + 1) {
                p1->pilot->enhancements[p1->pilot->har_id] = p2->pilot->enhancements[p1->pilot->har_id];
                fight_stats->plug_text = PLUG_ENHANCEMENT;
            } else if(hp_left_percent >= 75) {
                fight_stats->plug_text = PLUG_WIN_BIG + rand_int(3);
            } else if(hp_left_percent >= 50) {
                fight_stats->plug_text = PLUG_WIN_OK + rand_int(3);
            } else {
                fight_stats->plug_text = PLUG_WIN + rand_int(3);
            }
        } else if(p1->pilot->money < 0 && sell_highest_value_upgrade(p1->pilot, fight_stats->sold)) {
            fight_stats->plug_text = PLUG_SOLD_UPGRADE;
        } else if(warning_given && p1->pilot->money < 0) {
            fight_stats->plug_text = PLUG_KICK_OUT;
            p1->pilot->money = 0;
            sd_pilot_exit_tournament(p1->pilot);
        } else if(p1->pilot->money < 0) {
            fight_stats->plug_text = PLUG_WARNING;
        } else {
            fight_stats->plug_text = PLUG_LOSE + rand_int(5);
        }

        if(p1->chr && sg_save(p1->chr) != SD_SUCCESS) {
            log_error("Failed to save pilot %s", p1->chr->pilot.name);
        }
        if(is_demoplay(gs)) {
            game_state_set_next(gs, SCENE_VS);
        } else if(gs->match_settings.sim) {
            p2->pilot = NULL;
            game_state_set_next(gs, SCENE_MECHLAB);
        } else {
            game_state_set_next(gs, SCENE_NEWSROOM);
        }
    } else if(is_twoplayer(gs)) {
        game_state_set_next(gs, SCENE_MELEE);
    } else if(gs->net_mode == NET_MODE_LOBBY) {
        if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK) {
            net_controller_set_winner(game_state_get_player(scene->gs, 0)->ctrl, local->winner);
        }
        if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
            net_controller_set_winner(game_state_get_player(scene->gs, 1)->ctrl, local->winner);
        }
        game_state_set_next(gs, SCENE_LOBBY);
    } else {
        game_state_set_next(gs, SCENE_MELEE);
    }

    if(is_singleplayer(scene->gs) && winner_player_id == 0) {
        // cycle the maps in singleplayer
        scene->gs->arena++;
        if(scene->gs->arena > 4) {
            scene->gs->arena = 0;
        }
    }
}

void arena_reset(scene *sc) {
    arena_local *local = scene_get_userdata(sc);
    local->state = ARENA_STATE_STARTING;

    log_debug("resetting arena");

    // Kill all hazards and projectiles
    game_state_clear_objects(sc->gs, GROUP_PROJECTILE | GROUP_HAZARD | GROUP_SCRAP | GROUP_ANNOUNCEMENT);

    // Initial har data
    vec2i pos[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(HAR1_START_POS, ARENA_FLOOR);
    pos[1] = vec2i_create(HAR2_START_POS, ARENA_FLOOR);

    // init HARs
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(sc->gs, i);
        object *har_obj = game_state_find_object(sc->gs, game_player_get_har_obj_id(player));
        har_reset(har_obj);
        object_set_pos(har_obj, pos[i]);
        object_set_vel(har_obj, vec2f_create(0, 0));
        object_set_direction(har_obj, dir[i]);
        chr_score_clear_done(&player->score);
    }

    // wipe any tick timers
    ticktimer_close(&sc->tick_timer);
    ticktimer_init(&sc->tick_timer);

    // Set correct sounds for ready, round and number STL fields
    sc->bk_data->sound_translation_table[14] = 10;               // READY
    sc->bk_data->sound_translation_table[15] = 16;               // ROUND
    sc->bk_data->sound_translation_table[3] = 23 + local->round; // NUMBER

    if(local->rounds == 1) {
        // Start READY animation
        animation *ready_ani = &bk_get_info(sc->bk_data, 11)->ani;
        object *ready = omf_calloc(1, sizeof(object));
        object_create(ready, sc->gs, ready_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(ready, sc->bk_data->sound_translation_table);
        object_set_animation(ready, ready_ani);
        object_set_finish_cb(ready, scene_ready_anim_done);
        object_set_group(ready, GROUP_ANNOUNCEMENT);
        game_state_add_object(sc->gs, ready, RENDER_LAYER_TOP, 0, 0);
    } else {
        // ROUND animation
        animation *round_ani = &bk_get_info(sc->bk_data, 6)->ani;
        object *round = omf_calloc(1, sizeof(object));
        object_create(round, sc->gs, round_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(round, sc->bk_data->sound_translation_table);
        object_set_animation(round, round_ani);
        object_set_finish_cb(round, scene_ready_anim_done);
        object_set_group(round, GROUP_ANNOUNCEMENT);
        game_state_add_object(sc->gs, round, RENDER_LAYER_TOP, 0, 0);

        // Round number
        animation *number_ani = &bk_get_info(sc->bk_data, 7)->ani;
        object *number = omf_calloc(1, sizeof(object));
        object_create(number, sc->gs, number_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(number, sc->bk_data->sound_translation_table);
        object_set_animation(number, number_ani);
        object_select_sprite(number, local->round);
        object_set_sprite_override(number, 1);
        object_set_group(number, GROUP_ANNOUNCEMENT);
        game_state_add_object(sc->gs, number, RENDER_LAYER_TOP, 0, 0);
    }

    // When playing the Desert arena in Arcade mode, change
    // the palette each round to simulate time passing.
    if(sc->bk_data->file_id == 128 && (unsigned int)local->round < vector_size(&sc->bk_data->palettes)) {
        vga_state_set_base_palette_from_range(bk_get_palette(sc->bk_data, local->round), 0x60, 0x60, 0x40);
    }
}

int arena_is_over(scene *sc) {
    arena_local *local = scene_get_userdata(sc);

    if(local->over) {
        return local->winner;
    }

    return -1;
}

void arena_har_take_hit_hook(int hittee, af_move *move, scene *scene) {
    chr_score *score;
    chr_score *otherscore;
    object *hit_har;
    har *h;

    fight_stats *fight_stats = &scene->gs->fight_stats;
    // FIXME: Unsure how to calculate airborne attacks, so prevent underflow like this.
    int hitter = abs(hittee - 1);
    fight_stats->hits_landed[hitter]++;
    if(fight_stats->hits_landed[hitter] > fight_stats->total_attacks[hitter]) {
        fight_stats->total_attacks[hitter] = fight_stats->hits_landed[hitter];
    }

    if(hittee == 1) {
        score = game_player_get_score(game_state_get_player(scene->gs, 0));
        otherscore = game_player_get_score(game_state_get_player(scene->gs, 1));
        hit_har = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 1)));
    } else {
        score = game_player_get_score(game_state_get_player(scene->gs, 1));
        otherscore = game_player_get_score(game_state_get_player(scene->gs, 0));
        hit_har = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 0)));
    }
    h = hit_har->userdata;
    if(h->state == STATE_RECOIL) {
        log_debug("COMBO!");
    }
    bool no_points = is_demoplay(scene->gs) || !game_state_get_player(scene->gs, hitter)->selectable;
    chr_score_hit(score, no_points ? 0 : move->points);
    chr_score_interrupt(otherscore, object_get_pos(hit_har));
}

void arena_har_recover_hook(int player_id, scene *scene) {
    chr_score *score;
    object *o_har;

    if(player_id == 0) {
        score = game_player_get_score(game_state_get_player(scene->gs, 1));
        o_har = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 1)));
    } else {
        score = game_player_get_score(game_state_get_player(scene->gs, 0));
        o_har = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 0)));
    }

    chr_score_end_combo(score, object_get_pos(o_har));
}

void arena_har_hit_wall_hook(int player_id, int wall, scene *scene) {
    object *o_har =
        game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, player_id)));
    har *h = object_get_userdata(o_har);

    // log_debug("Player %d hit wall %d", player_id, wall);

    // Don't allow object to collide if it is being grabbed.
    if(h->is_grabbed) {
        return;
    }

    // HAR must be in the air to be get faceplanted to a wall.
    if(o_har->pos.y >= ARENA_FLOOR - 10) {
        // TODO: Grounded desert wall logic

        return;
    }

    float abs_velocity_h = fabsf(o_har->vel.x) / o_har->horizontal_velocity_modifier;

    if(abs_velocity_h > 2) {
        int tolerance = arena_get_wall_slam_tolerance(scene->gs);

        // log_debug("Checking if %f velocity will wallslam", abs_velocity_h);
        if((abs_velocity_h + 0.5f) > tolerance && (h->state == STATE_FALLEN || h->state == STATE_RECOIL)) {
            h->state = STATE_WALLDAMAGE;

            bk_info *info = bk_get_info(scene->bk_data, 20 + wall);
            if(info) { // Only Power Plant and Desert have wall animations
                object *obj = omf_calloc(1, sizeof(object));
                object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0, 0));
                object_set_stl(obj, scene->bk_data->sound_translation_table);
                object_set_animation(obj, &info->ani);
                if(game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM, 1, 0) != 0) {
                    object_free(obj);
                    omf_free(obj);
                }
            }

            // spawn the electricity on top of the HAR
            // TODO this doesn't track the har's position well...
            info = bk_get_info(scene->bk_data, 22);
            if(info) { // Only Power Plant has the electric overlay effect
                object *obj2 = omf_calloc(1, sizeof(object));
                object_create(obj2, scene->gs, vec2i_create(o_har->pos.x, o_har->pos.y), vec2f_create(0, 0));
                object_set_stl(obj2, scene->bk_data->sound_translation_table);
                object_set_animation(obj2, &info->ani);
                object_attach_to(obj2, o_har);
                // object_dynamic_tick(obj2);
                if(game_state_add_object(scene->gs, obj2, RENDER_LAYER_TOP, 0, 0)) {
                    object_free(obj2);
                    omf_free(obj2);
                }
            }

            int amount = rand_int(2) + 3;
            for(int i = 0; i < amount; i++) {
                int variance = rand_int(20) - 10;
                int anim_no = rand_int(2) + 24;
                // log_debug("XXX anim = %d, variance = %d", anim_no, variance);
                int pos_y = o_har->pos.y - object_get_size(o_har).y + variance + i * 25;
                vec2i coord = vec2i_create(o_har->pos.x, pos_y);
                object *dust = omf_calloc(1, sizeof(object));
                object_create(dust, scene->gs, coord, vec2f_create(0, 0));
                object_set_stl(dust, scene->bk_data->sound_translation_table);
                object_set_animation(dust, &bk_get_info(scene->bk_data, anim_no)->ani);
                game_state_add_object(scene->gs, dust, RENDER_LAYER_MIDDLE, 0, 0);
            }

            // Wallhit sound
            float d = ((float)o_har->pos.x) / 640.0f;
            float pos_pan = d - 0.25f;
            game_state_play_sound(o_har->gs, 68, 1.0f, pos_pan, 2.0f);

            // Set hit animation
            object_set_animation(o_har, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
            object_set_repeat(o_har, 0);
            scene->gs->screen_shake_horizontal = 3 * fabsf(o_har->vel.x);
            // from MASTER.DAT
            object_set_custom_string(o_har, "hQ1-hQ7-x-3Q5-x-2L5-x-2M900");

            if(wall == 1) {
                o_har->pos.x = ARENA_RIGHT_WALL - 2;
                object_set_direction(o_har, OBJECT_FACE_RIGHT);
            } else {
                o_har->pos.x = ARENA_LEFT_WALL + 2;
                object_set_direction(o_har, OBJECT_FACE_LEFT);
            }
        } else {
            bk_info *info = bk_get_info(scene->bk_data, 20 + wall);
            if(info && (info->hazard_damage == 0)) {
                object *obj = omf_calloc(1, sizeof(object));
                object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0, 0));
                object_set_stl(obj, scene->bk_data->sound_translation_table);
                object_set_animation(obj, &info->ani);
                // TODO: Adjust this animation based on velocity
                object_set_custom_string(obj, "brwA1-brwB1-brwD1-brwE0-brwD4-brwC2-brwB2-brwA2");
                if(game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM, 1, 0) != 0) {
                    object_free(obj);
                    omf_free(obj);
                }
            }
        }
    }
}

void arena_har_defeat_hook(int loser_player_id, scene *scene) {
    game_state *gs = scene->gs;
    arena_local *local = scene_get_userdata(scene);
    int winner_player_id = abs(loser_player_id - 1);
    game_player *player_winner = game_state_get_player(scene->gs, winner_player_id);
    game_player *player_loser = game_state_get_player(scene->gs, loser_player_id);
    object *winner = game_state_find_object(scene->gs, game_player_get_har_obj_id(player_winner));
    object *loser = game_state_find_object(scene->gs, game_player_get_har_obj_id(player_loser));
    har *winner_har = object_get_userdata(winner);
    fight_stats *fight_stats = &gs->fight_stats;
    fight_stats->winner = winner_player_id;
    chr_score *score = game_player_get_score(game_state_get_player(gs, winner_player_id));
    // XXX need a smarter way to detect if a player is networked or local
    if(player_winner->ctrl->type != CTRL_TYPE_NETWORK && player_loser->ctrl->type == CTRL_TYPE_NETWORK) {
        local->win_state = YOUWIN;
    } else if(player_winner->ctrl->type == CTRL_TYPE_NETWORK && player_loser->ctrl->type != CTRL_TYPE_NETWORK) {
        local->win_state = YOULOSE;
    } else {
        if(is_demoplay(gs)) {
            // in demo mode, "you lose" should always be displayed
            local->win_state = YOULOSE;
        } else if(!is_singleplayer(gs)) {
            // XXX in two player mode, "you win" should always be displayed
            local->win_state = YOUWIN;
        } else {
            if(loser_player_id == 1) {
                local->win_state = YOUWIN;
            } else {
                local->win_state = YOULOSE;
            }
        }
    }
    object *round_token = game_state_find_object(gs, local->player_rounds[winner_player_id][score->rounds]);
    if(round_token) {
        object_set_sprite_override(round_token, 0);
        object_select_sprite(round_token, 0);
        object_set_sprite_override(round_token, 1);
    }
    score->rounds++;
    if(player_winner->ctrl->type != CTRL_TYPE_AI && player_loser->ctrl->type == CTRL_TYPE_AI) {
        chr_score_victory(score, har_health_percent(winner_har));
    }
    if(score->rounds >= ceilf(local->rounds / 2.0f)) {
        winner_har->state = STATE_VICTORY;
        if(winner_har->executing_move == 0 && defeated_at_rest(loser)) {
            har_face_enemy(winner, loser);
            har_set_ani(winner, ANIM_VICTORY, 0);
        }
        winner_har->enqueued = 0;
        local->over = 1;
        local->winner = winner_player_id;
        if(is_singleplayer(gs)) {
            player_winner->sp_wins |= 2 << player_loser->pilot->pilot_id;
            if(player_loser->pilot->pilot_id == PILOT_KREISSACK) {
                har *loser_har = object_get_userdata(loser);
                log_debug("kreissack defeated");
                // can't scrap/destruct kreissack
                winner_har->state = STATE_DONE;
                // major go boom
                loser_har->custom_defeat_animation = 47;
            }
        }
    } else {
        // can't do scrap/destruct except on final round
        winner_har->state = STATE_DONE;
    }
}

void arena_maybe_turn_har(int player_id, scene *scene) {
    int other_player_id = abs(player_id - 1);
    object *obj_har1 =
        game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, player_id)));
    object *obj_har2 = game_state_find_object(
        scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, other_player_id)));

    har_face_enemy(obj_har1, obj_har2);
}

void arena_har_hook(har_event event, void *data) {
    scene *scene = data;
    fight_stats *fight_stats = &scene->gs->fight_stats;
    int other_player_id = abs(event.player_id - 1);
    arena_local *arena = scene_get_userdata(scene);
    chr_score *score = game_player_get_score(game_state_get_player(scene->gs, event.player_id));
    object *obj_har1 = game_state_find_object(
        scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, event.player_id)));
    object *obj_har2 = game_state_find_object(
        scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, other_player_id)));
    har *har1 = obj_har1->userdata;
    har *har2 = obj_har2->userdata;
    switch(event.type) {
        case HAR_EVENT_WALK:
            arena_maybe_turn_har(event.player_id, scene);
            break;
        case HAR_EVENT_AIR_TURN:
            arena_maybe_turn_har(event.player_id, scene);
            break;
        case HAR_EVENT_TAKE_HIT:
        case HAR_EVENT_TAKE_HIT_PROJECTILE:
            if(af_get_move(har2->af_data, obj_har2->cur_animation->id)->category != CAT_CLOSE) {
                arena_maybe_turn_har(event.player_id, scene);
            }
            arena_har_take_hit_hook(event.player_id, event.move, scene);
            break;
        case HAR_EVENT_HIT_WALL:
            arena_har_hit_wall_hook(event.player_id, event.wall, scene);
            break;
        case HAR_EVENT_ATTACK:
            fight_stats->total_attacks[event.player_id]++;
            if(object_is_airborne(obj_har1)) {
                har1->air_attacked = 1;
                log_debug("AIR ATTACK %u", event.player_id);
            } else {
                // XXX this breaks the backwards razor spin and anything else using the 'ar' tag, so lets disable it for
                // now
                // arena_maybe_turn_har(event.player_id, scene);
            }
            break;
        case HAR_EVENT_LAND:
            if(har2->state == STATE_STANDING || har_is_crouching(har2) || har_is_walking(har2) ||
               har2->executing_move) {
                // if the other HAR is jumping or recoiling, don't flip the direction. This specifically is to fix
                // jaguar ending up facing backwards after an overhead throw.
                arena_maybe_turn_har(other_player_id, scene);
            }
            log_debug("LAND %u", event.player_id);
            break;
        case HAR_EVENT_AIR_ATTACK_DONE:
            har1->air_attacked = 0;
            log_debug("AIR_ATTACK_DONE %u", event.player_id);
            break;
        case HAR_EVENT_RECOVER:
            arena_har_recover_hook(event.player_id, scene);
            if(!object_is_airborne(obj_har1)) {
                arena_maybe_turn_har(event.player_id, scene);
                log_debug("RECOVER %u", event.player_id);
            }
            break;
        case HAR_EVENT_DEFEAT:
            if(arena->state != ARENA_STATE_ENDING) {
                arena->ending_ticks = 0;
                arena->state = ARENA_STATE_ENDING;
                arena_har_defeat_hook(event.player_id, scene);
            }
            break;
        case HAR_EVENT_SCRAP:
            scene->gs->fight_stats.finish = FINISH_SCRAP;
            chr_score_scrap(score);
            break;
        case HAR_EVENT_DESTRUCTION:
            scene->gs->fight_stats.finish = FINISH_DESTRUCTION;
            chr_score_destruction(score);
            log_debug("DESTRUCTION!");
            break;
        case HAR_EVENT_DONE:
            chr_score_done(score);
            log_debug("DONE!");
            break;
    }
}

void maybe_install_har_hooks(scene *scene) {
    object *obj_har1, *obj_har2;
    obj_har1 = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 0)));
    obj_har2 = game_state_find_object(scene->gs, game_player_get_har_obj_id(game_state_get_player(scene->gs, 1)));
    har *har1, *har2;
    har1 = obj_har1->userdata;
    har2 = obj_har2->userdata;

    har_install_hook(har1, &arena_har_hook, scene);
    har_install_hook(har2, &arena_har_hook, scene);
}

// djb hash
uint32_t arena_state_hash(game_state *gs) {
    uint32_t hash = 5381;
    // include the arena ID
    hash = ((hash << 5) + hash) + gs->sc->id;
    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(gs, i);
        object *obj_har = game_state_find_object(gs, game_player_get_har_obj_id(player));
        har *har = obj_har->userdata;
        vec2i pos = object_get_pos(obj_har);
        vec2f vel = object_get_vel(obj_har);
        uint32_t x = (uint32_t)pos.x;
        uint32_t y = (uint32_t)pos.y;
        uint32_t health = (uint32_t)har->health;
        uint32_t endurance = (uint32_t)har->endurance;
        hash = ((hash << 5) + hash) + har->id;
        hash = ((hash << 5) + hash) + player->pilot->power;
        hash = ((hash << 5) + hash) + player->pilot->agility;
        hash = ((hash << 5) + hash) + player->pilot->endurance;
        hash = ((hash << 5) + hash) + x;
        hash = ((hash << 5) + hash) + y;
        hash = ((hash << 5) + hash) + health;
        hash = ((hash << 5) + hash) + endurance;
        hash = ((hash << 5) + hash) + (uint32_t)vel.x;
        // we are inconsistent on applying gravity
        // hash = ((hash << 5) + hash) + (uint32_t)vel.y;
        hash = ((hash << 5) + hash) + har->state;
        hash = ((hash << 5) + hash) + har->executing_move;
        hash = ((hash << 5) + hash) + obj_har->cur_animation->id;
    }
    return hash;
}

char *state_name(int state) {
    switch(state) {
        case STATE_STANDING:
            return "standing";
        case STATE_WALKTO:
            return "walk_to";
        case STATE_WALKFROM:
            return "walk_from";
        case STATE_CROUCHING:
            return "crouching";
        case STATE_CROUCHBLOCK:
            return "crouchblock";
        case STATE_JUMPING:
            return "jumping";
        case STATE_RECOIL:
            return "recoil";
        case STATE_FALLEN:
            return "fallen";
        case STATE_STANDING_UP:
            return "standing_up";
        case STATE_STUNNED:
            return "stunned";
        case STATE_BLOCKSTUN:
            return "blockstun";
        case STATE_VICTORY:
            return "victory";
        case STATE_DEFEAT:
            return "defeat";
        case STATE_SCRAP:
            return "scrap";
        case STATE_DESTRUCTION:
            return "destruction";
        case STATE_WALLDAMAGE: // Took damage from wall (electrocution)
            return "wall_damage";
        case STATE_DONE: // destruction or scrap has completed
            return "done";
        default:
            return "unknown!!!";
    }
}

void arena_state_dump(game_state *gs, char *buf, size_t bufsize) {
    int off = 0;
    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(gs, i);
        object *obj_har = game_state_find_object(gs, game_player_get_har_obj_id(player));
        har *har = obj_har->userdata;
        vec2i pos = object_get_pos(obj_har);
        vec2f vel = object_get_vel(obj_har);
        off = snprintf(buf + off, bufsize - off,
                       "player %d  power %d agility %d endurance %d HAR id %d  pos %d,%d, health %d, endurance %f, "
                       "velocity %f,%f, state %s, executing_move %d cur_anim %d\n",
                       i, player->pilot->power, player->pilot->agility, player->pilot->endurance, har->id, pos.x, pos.y,
                       har->health, (float)har->endurance, vel.x, vel.y, state_name(har->state), har->executing_move,
                       obj_har->cur_animation->id);
    }
}

// -------- Scene callbacks --------

void arena_free(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    game_state_set_paused(scene->gs, 0);

    if(scene->gs->rec && scene->gs->init_flags->playback == 0) {
        sd_rec_finish(scene->gs->rec, scene->gs->int_tick);

        if(scene->gs->init_flags->record == 1) {
            // we're supposed to save it
            sd_rec_save(scene->gs->rec, scene->gs->init_flags->rec_file);
            sd_rec_free(scene->gs->rec);
            omf_free(scene->gs->rec);
            scene->gs->rec = NULL;
        }
    }

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_har(player, NULL);
        // game_player_set_ctrl(player, NULL);
        controller_set_repeat(game_player_get_ctrl(player), 0);
    }

    gui_frame_free(local->game_menu);

    audio_stop_music();

    // Free bar components
    for(int i = 0; i < 2; i++) {
        component_free(local->health_bars[i]);
        component_free(local->endurance_bars[i]);
    }

    settings_save();

    omf_free(local);
    scene_set_userdata(scene, local);
}

void arena_clone_free(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    omf_free(local);
}

void write_rec_move(scene *scene, game_player *player, int action) {
    arena_local *local = scene_get_userdata(scene);
    sd_rec_move move;
    if(!scene->gs->rec) {
        return;
    }

    memset(&move, 0, sizeof(move));
    move.tick = scene->gs->tick;
    move.lookup_id = 2;
    move.player_id = 0;
    move.action = 0;

    if(player == game_state_get_player(scene->gs, 1)) {
        move.player_id = 1;
    }

    if(action & ACT_PUNCH) {
        move.action |= SD_ACT_PUNCH;
    }

    if(action & ACT_KICK) {
        move.action |= SD_ACT_KICK;
    }

    if(action & ACT_UP) {
        move.action |= SD_ACT_UP;
    }

    if(action & ACT_DOWN) {
        move.action |= SD_ACT_DOWN;
    }

    if(action & ACT_LEFT) {
        move.action |= SD_ACT_LEFT;
    }

    if(action & ACT_RIGHT) {
        move.action |= SD_ACT_RIGHT;
    }

    if(local->rec_last[move.player_id] == move.action) {
        return;
    }
    local->rec_last[move.player_id] = move.action;

    int ret;

    if((ret = sd_rec_insert_action(scene->gs->rec, scene->gs->rec->move_count, &move)) != SD_SUCCESS) {
        log_debug("recoding move failed %d", ret);
    }
}

int arena_handle_events(scene *scene, game_player *player, ctrl_event *i) {
    int need_sync = 0;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                need_sync += object_act(game_state_find_object(scene->gs, game_player_get_har_obj_id(player)),
                                        i->event_data.action);

                if(!is_netplay(scene->gs)) {
                    // netplay will manage its own REC events
                    write_rec_move(scene, player, i->event_data.action);
                }
            } else if(i->type == EVENT_TYPE_CLOSE) {
                if(player->ctrl->type == CTRL_TYPE_REC) {
                    game_state_set_next(scene->gs, SCENE_NONE);
                } else {
                    if(scene->gs->net_mode == NET_MODE_LOBBY) {
                        arena_local *local = scene_get_userdata(scene);
                        if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK) {
                            net_controller_set_winner(game_state_get_player(scene->gs, 0)->ctrl, local->winner);
                        }
                        if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
                            net_controller_set_winner(game_state_get_player(scene->gs, 1)->ctrl, local->winner);
                        }
                        game_state_set_next(scene->gs, SCENE_LOBBY);
                    }
                    game_state_set_next(scene->gs, SCENE_MENU);
                }
                return 0;
            }
        } while((i = i->next));
    }
    return need_sync;
}

void arena_spawn_hazard(scene *scene) {
    iterator it;
    hashmap_iter_begin(&scene->bk_data->infos, &it);
    hashmap_pair *pair = NULL;

    foreach(it, pair) {
        bk_info *info = (bk_info *)pair->value;
        if(info->probability > 1) {
            if(random_int(&scene->gs->rand, info->probability) == 1) {
                // TODO don't spawn it if we already have this animation running
                object *obj = omf_calloc(1, sizeof(object));
                object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0, 0));
                object_set_stl(obj, scene->bk_data->sound_translation_table);
                object_set_animation(obj, &info->ani);
                if(scene->id == SCENE_ARENA3 && info->ani.id == 0) {
                    // XXX fire pit orb has a bug whwre it double spawns. Use a custom animation string to avoid it
                    // it mioght be to do with the 'mp' tag, which we don't currently understand
                    object_set_custom_string(obj, "Z3-mx+160my+100m15mp10Z1-Z300");
                }
                /*object_set_spawn_cb(obj, cb_scene_spawn_object, (void*)scene);*/
                /*object_set_destroy_cb(obj, cb_scene_destroy_object, (void*)scene);*/
                hazard_create(obj, scene);
                if(game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM, 1, 0) == 0) {
                    object_set_layers(obj, LAYER_HAZARD | LAYER_HAR);
                    object_set_group(obj, GROUP_HAZARD);
                    object_set_userdata(obj, scene->bk_data);
                    if(info->ani.extra_string_count > 0) {
                        // For the desert, there's a bunch of extra animation strgins for
                        // the different plane formations.
                        // Pick one, rather than always use the first

                        int r = random_int(&scene->gs->rand, info->ani.extra_string_count);
                        if(r > 0) {
                            str *s = vector_get(&info->ani.extra_strings, r);
                            object_set_custom_string(obj, str_c(s));
                        }
                    }

                    log_debug("Arena tick: Hazard with probability %d started.", info->probability, info->ani.id);
                } else {
                    object_free(obj);
                    omf_free(obj);
                }
            }
        }
    }
}

bool har_in_defeat_animation(object *obj) {
    har *h = obj->userdata;
    return obj->cur_animation->id == (h->custom_defeat_animation ? h->custom_defeat_animation : ANIM_DEFEAT);
}

bool defeated_at_rest(object *obj) {
    return har_in_defeat_animation(obj) && !object_is_airborne(obj) && obj->vel.x == 0.0f;
}

bool har_unfinished_victory(object *obj) {
    har *h = obj->userdata;
    if(h->walk_destination > 0 && h->walk_done_anim) {
        return true;
    }
    return obj->cur_animation->id == ANIM_VICTORY && !player_is_last_frame(obj) && !player_is_looping(obj);
}

bool winner_needs_victory_pose(object *obj) {
    har *h = obj->userdata;
    return !object_is_airborne(obj) && (h->state == STATE_DONE || h->state == STATE_VICTORY) &&
           obj->cur_animation->id != ANIM_VICTORY;
}

void arena_dynamic_tick(scene *scene, int paused) {
    arena_local *local = scene_get_userdata(scene);
    game_state *gs = scene->gs;

    if(!paused) {
        object *obj_har[2];
        har *hars[2];
        for(int i = 0; i < 2; i++) {
            obj_har[i] = game_state_find_object(gs, game_player_get_har_obj_id(game_state_get_player(gs, i)));
            hars[i] = obj_har[i]->userdata;
        }

        // Handle scrolling score texts
        chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 0)));
        chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 1)));

        // Set and tick all proggressbars
        for(int i = 0; i < 2; i++) {
            float hp = (float)hars[i]->health / (float)hars[i]->health_max;
            float en = (float)hars[i]->endurance / (float)hars[i]->endurance_max;
            progressbar_set_progress(local->health_bars[i], hp * 100, !(gs->warp_speed || gs->clone));
            progressbar_set_progress(local->endurance_bars[i], en * 100, !(gs->warp_speed || gs->clone));
            progressbar_set_flashing(local->endurance_bars[i], (en * 100 < 50), 8);
            component_tick(local->health_bars[i]);
            component_tick(local->endurance_bars[i]);
        }

        // Endings and beginnings
        if(local->state != ARENA_STATE_ENDING && local->state != ARENA_STATE_STARTING) {
            if(scene->gs->match_settings.hazards) {
                arena_spawn_hazard(scene);
            }
        }
        if(local->state == ARENA_STATE_ENDING) {
            // check if its time to put the winner into victory pose
            if(defeated_at_rest(obj_har[0]) && winner_needs_victory_pose(obj_har[1])) {
                har_face_enemy(obj_har[1], obj_har[0]);
                har_set_ani(obj_har[1], ANIM_VICTORY, 0);
            } else if(defeated_at_rest(obj_har[1]) && winner_needs_victory_pose(obj_har[0])) {
                har_face_enemy(obj_har[0], obj_har[1]);
                har_set_ani(obj_har[0], ANIM_VICTORY, 0);
            }

            chr_score *s1 = game_player_get_score(game_state_get_player(scene->gs, 0));
            chr_score *s2 = game_player_get_score(game_state_get_player(scene->gs, 1));
            if(local->win_state && local->win_state != DONE &&
               (defeated_at_rest(obj_har[0]) || defeated_at_rest(obj_har[1]))) {
                if(local->win_state == YOULOSE) {
                    scene_youlose_anim_start(scene->gs);
                } else if(local->win_state == YOUWIN) {
                    scene_youwin_anim_start(scene->gs);
                }
                local->win_state = NONE;
            } else if(local->win_state == DONE) {
                // you win/lose animation is done
                if(player_frame_isset(obj_har[0], "be") || player_frame_isset(obj_har[1], "be") ||
                   chr_score_onscreen(s1) || chr_score_onscreen(s2) || har_unfinished_victory(obj_har[0]) ||
                   har_unfinished_victory(obj_har[1])) {
                } else {
                    local->ending_ticks++;
                }
            }
            if(local->ending_ticks == 18) {
                arena_screengrab_winner(scene);
            }

            if(local->ending_ticks == 40) {
                // one HAR must be in victory pose and one must be in defeat or damage from scrap/destruction
                assert(((obj_har[0]->cur_animation->id == ANIM_VICTORY ||
                         af_get_move(hars[0]->af_data, obj_har[0]->cur_animation->id)->category == CAT_SCRAP ||
                         af_get_move(hars[0]->af_data, obj_har[0]->cur_animation->id)->category == CAT_DESTRUCTION) &&
                        (har_in_defeat_animation(obj_har[1]) || obj_har[1]->cur_animation->id == ANIM_DAMAGE)) ||
                       ((obj_har[1]->cur_animation->id == ANIM_VICTORY ||
                         af_get_move(hars[0]->af_data, obj_har[0]->cur_animation->id)->category == CAT_SCRAP ||
                         af_get_move(hars[0]->af_data, obj_har[0]->cur_animation->id)->category == CAT_DESTRUCTION) &&
                        (har_in_defeat_animation(obj_har[0]) || obj_har[1]->cur_animation->id == ANIM_DAMAGE)));
                if(!local->over) {
                    local->round++;
                    arena_reset(scene);
                } else {
                    arena_end(scene);
                }
            }
        }

        // Pour some rein!
        if(local->rein_enabled) {
            if(rand_float() > 0.65f) {
                vec2i pos = vec2i_create(rand_int(NATIVE_W), -10);
                for(int harnum = 0; harnum < game_state_num_players(gs); harnum++) {
                    object *h_obj = game_state_find_object(gs, game_state_get_player(gs, harnum)->har_obj_id);
                    har *h = object_get_userdata(h_obj);
                    // Calculate velocity etc.
                    float rv = rand_float() - 0.5f;
                    float velx = rv;
                    float vely = -12 * sinf(0 / 2 + rv);

                    // Make sure scrap has somekind of velocity
                    // (to prevent floating scrap objects)
                    if(vely < 0.1f && vely > -0.1f)
                        vely += 0.21f;

                    // Create the object
                    object *scrap = omf_calloc(1, sizeof(object));
                    int anim_no = rand_int(3) + ANIM_SCRAP_METAL;
                    object_create(scrap, gs, pos, vec2f_create(velx, vely));
                    object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
                    object_set_gravity(scrap, 0.4f);
                    object_set_pal_offset(scrap, object_get_pal_offset(h_obj));
                    object_set_pal_limit(scrap, object_get_pal_limit(h_obj));
                    object_set_layers(scrap, LAYER_SCRAP);
                    object_set_shadow(scrap, 1);
                    object_set_group(scrap, GROUP_SCRAP);
                    object_dynamic_tick(scrap);
                    scrap_create(scrap);
                    game_state_add_object(gs, scrap, RENDER_LAYER_TOP, 0, 0);
                }
            }
        }

        // check some invariants
        assert(obj_har[0]->pos.x >= ARENA_LEFT_WALL && obj_har[0]->pos.x <= ARENA_RIGHT_WALL);
        assert(obj_har[1]->pos.x >= ARENA_LEFT_WALL && obj_har[1]->pos.x <= ARENA_RIGHT_WALL);
        if(hars[0]->health == 0) {
            assert(hars[0]->state == STATE_DEFEAT || hars[0]->state == STATE_RECOIL || hars[0]->state == STATE_FALLEN ||
                   hars[0]->state == STATE_NONE || hars[0]->state == STATE_WALLDAMAGE);
        }
        if(hars[1]->health == 0) {
            assert(hars[1]->state == STATE_DEFEAT || hars[1]->state == STATE_RECOIL || hars[1]->state == STATE_FALLEN ||
                   hars[1]->state == STATE_NONE || hars[1]->state == STATE_WALLDAMAGE);
        }
    } // if(!paused)
}

void arena_static_tick(scene *scene, int paused) {
    arena_local *local = scene_get_userdata(scene);
    gui_frame_tick(local->game_menu);
}

void arena_input_tick(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    if(!game_state_is_paused(scene->gs)) {
        game_player *player1 = game_state_get_player(scene->gs, 0);
        game_player *player2 = game_state_get_player(scene->gs, 1);

        ctrl_event *p1 = NULL, *p2 = NULL;
        controller_poll(player1->ctrl, &p1);
        controller_poll(player2->ctrl, &p2);

        arena_handle_events(scene, player1, p1);
        arena_handle_events(scene, player2, p2);
        controller_free_chain(p1);
        controller_free_chain(p2);
    }

    if(is_netplay(scene->gs)) {
        // no pause menu during netplay
        return;
    }

    ctrl_event *menu_ev = NULL;
    game_state_menu_poll(scene->gs, &menu_ev);
    for(ctrl_event *i = menu_ev; i; i = i->next) {
        if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_ESC && is_demoplay(scene->gs)) {
            // exit demoplay
            game_state_set_next(scene->gs, SCENE_MENU);
        } else if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_ESC) {
            // toggle menu
            local->menu_visible = !local->menu_visible;
            game_state_set_paused(scene->gs, local->menu_visible);
        } else if(i->type == EVENT_TYPE_ACTION && local->menu_visible && i->event_data.action != ACT_ESC) {
            // menu events
            gui_frame_action(local->game_menu, i->event_data.action);
        }
    }
    controller_free_chain(menu_ev);
}

int arena_event(scene *scene, SDL_Event *e) {
    return 0;
}

void arena_render_overlay(scene *scene) {
    if(scene->gs->hide_ui) {
        return;
    }

    arena_local *local = scene_get_userdata(scene);

    // Render bars
    game_player *player[2];
    object *obj[2];

    char buf[40];

    text_settings tconf_players;
    text_defaults(&tconf_players);
    tconf_players.font = FONT_SMALL;
    tconf_players.cforeground = TEXT_HUD_COLOR;
    tconf_players.cshadow = TEXT_HUD_SHADOW;
    tconf_players.shadow = TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM;
    const font *fnt = fonts_get_font(tconf_players.font);

    for(int i = 0; i < 2; i++) {
        player[i] = game_state_get_player(scene->gs, i);
        obj[i] = game_state_find_object(scene->gs, game_player_get_har_obj_id(player[i]));
    }
    if(obj[0] != NULL && obj[1] != NULL) {
        //  Render progress bar components
        for(int i = 0; i < 2; i++) {
            component_render(local->health_bars[i]);
            component_render(local->endurance_bars[i]);
        }

        // Render HAR and pilot names
        const char *player1_name = NULL;
        const char *player2_name = NULL;
        player1_name = player[0]->pilot->name;
        if(player[1]->pilot) {
            // when quitting this can go null
            player2_name = player[1]->pilot->name;
        }

        text_render_mode(&tconf_players, TEXT_DEFAULT, 5, 19, 250, 6, player1_name);
        text_render_mode(&tconf_players, TEXT_DEFAULT, 5, 26, 250, 6, lang_get((player[0]->pilot->har_id) + 31));

        // when quitting, pilot can go null
        if(player[1]->pilot) {
            char const *player2_har = lang_get((player[1]->pilot->har_id) + 31);
            int p2len = strlen(player2_name) * fnt->w;
            int h2len = (strlen(player2_har) - 1) * fnt->w; // TODO: FIXME(lang): -1 because of errant newline
            text_render_mode(&tconf_players, TEXT_DEFAULT, 315 - p2len, 19, 100, 6, player2_name);
            text_render_mode(&tconf_players, TEXT_DEFAULT, 315 - h2len, 26, 100, 6, player2_har);
        }

        // Render score stuff
        chr_score_render(game_player_get_score(player[0]), game_player_get_selectable(player[0]));
        chr_score_render(game_player_get_score(player[1]), game_player_get_selectable(player[1]));

        // render ping, if player is networked
        if(player[0]->ctrl->type == CTRL_TYPE_NETWORK) {
            snprintf(buf, 40, "ping %d", player[0]->ctrl->rtt / 2);
            text_render_mode(&tconf_players, TEXT_DEFAULT, 5, 40, 250, 6, buf);
        }

        if(player[1]->ctrl->type == CTRL_TYPE_NETWORK) {
            snprintf(buf, 40, "ping %d", player[1]->ctrl->rtt / 2);
            text_render_mode(&tconf_players, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 40, 250, 6, buf);
        }
    }

    // Render menu (if visible)
    if(local->menu_visible) {
        gui_frame_render(local->game_menu);
    }
}

static void arena_debug(scene *scene) {
    if(scene->gs->hide_ui) {
        return;
    }

    char buf[100];
    game_player *player[2];
    object *obj_har[2];
    har *hars[2];
    for(int i = 0; i < 2; i++) {
        player[i] = game_state_get_player(scene->gs, i);
        obj_har[i] = game_state_find_object(scene->gs, game_player_get_har_obj_id(player[i]));
        hars[i] = obj_har[i]->userdata;
    }

    text_settings tconf_debug;
    text_defaults(&tconf_debug);
    tconf_debug.font = FONT_SMALL;
    tconf_debug.cforeground = TEXT_HUD_COLOR;
    tconf_debug.cshadow = TEXT_HUD_SHADOW;

    const font *fnt = fonts_get_font(tconf_debug.font);

    snprintf(buf, 40, "%u", game_state_get_tick(scene->gs));
    text_render_mode(&tconf_debug, TEXT_DEFAULT, 160, 0, 250, 6, buf);
    snprintf(buf, 40, "%u", random_get_seed(&scene->gs->rand));
    text_render_mode(&tconf_debug, TEXT_DEFAULT, 130, 8, 250, 6, buf);

    if(player[0]->ctrl->type == CTRL_TYPE_AI) {
        ai_controller_print_state(player[0]->ctrl, buf, sizeof(buf));
        text_render_mode(&tconf_debug, TEXT_DEFAULT, 5, 40, 250, 6, buf);
    }

    if(player[1]->ctrl->type == CTRL_TYPE_AI) {
        ai_controller_print_state(player[1]->ctrl, buf, sizeof(buf));
        text_render_mode(&tconf_debug, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 40, 250, 6, buf);
    }

    for(int i = 0; i < 2; i++) {

        snprintf(buf, sizeof(buf), "%d", hars[i]->health);
        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 90, 5, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 230 - (strlen(buf) * fnt->w), 5, 250, 6, buf);
        }

        snprintf(buf, sizeof(buf), "%.2f", hars[i]->endurance);
        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 70, 12, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 250 - (strlen(buf) * fnt->w), 12, 250, 6, buf);
        }

        snprintf(buf, sizeof(buf), "%s(%d)", state_name(hars[i]->state), hars[i]->state);
        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 5, 48, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 48, 250, 6, buf);
        }

        snprintf(buf, sizeof(buf), "pos: %.3f %.3f", obj_har[i]->pos.x, obj_har[i]->pos.y);

        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 5, 56, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 56, 250, 6, buf);
        }

        snprintf(buf, sizeof(buf), "vel: %.3f %.3f", obj_har[i]->vel.x, obj_har[i]->vel.y);

        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 5, 62, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 62, 250, 6, buf);
        }

        snprintf(buf, sizeof(buf), "aa: %d em: %d ani: %d", hars[i]->air_attacked, hars[i]->executing_move,
                 obj_har[i]->cur_animation->id);

        if(i == 0) {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 5, 70, 250, 6, buf);
        } else {
            text_render_mode(&tconf_debug, TEXT_DEFAULT, 315 - (strlen(buf) * fnt->w), 70, 250, 6, buf);
        }
    }
}

int arena_get_state(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    return local->state;
}

void arena_set_state(scene *scene, int state) {
    arena_local *local = scene_get_userdata(scene);
    local->state = state;
}

void arena_toggle_rein(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    local->rein_enabled = !local->rein_enabled;
}

void arena_clone(scene *src, scene *dst) {
    arena_local *local = omf_calloc(1, sizeof(arena_local));
    dst->userdata = local;
    memcpy(dst->userdata, src->userdata, sizeof(arena_local));
    maybe_install_har_hooks(dst);

    component *c = gui_frame_find(local->game_menu, GAME_MENU_QUIT_ID);
    button_set_userdata(c, dst);
    c = gui_frame_find(local->game_menu, GAME_MENU_RETURN_ID);
    button_set_userdata(c, dst);
}

int arena_get_wall_slam_tolerance(game_state *gs) {
    if(gs->match_settings.hazards) {
        int arena_id = gs->this_id - SCENE_ARENA0;
        if(arena_id == 2) {
            return wall_slam_tolerance_powerplant;
        }
    }

    return wall_slam_tolerance_default;
}

void arena_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    if(scene->bk_data->file_id == 64) {
        // Start up & repeat torches on arena startup
        switch(id) {
            case 1:
            case 2:
            case 3:
            case 4:
                *m_load = 1;
                *m_repeat = 1;
                return;
        }
    }
}

int arena_create(scene *scene) {
    settings *setting;
    arena_local *local;

    // Load up settings
    setting = settings_get();

    fight_stats *fight_stats = &scene->gs->fight_stats;
    memset(fight_stats, 0, sizeof(*fight_stats));

    // Handle music playback
    switch(scene->bk_data->file_id) {
        case 8:
            audio_play_music(PSM_ARENA0);
            break;
        case 16:
            audio_play_music(PSM_ARENA1);
            break;
        case 32:
            audio_play_music(PSM_ARENA2);
            break;
        case 64:
            audio_play_music(PSM_ARENA3);
            break;
        case 128:
            audio_play_music(PSM_ARENA4);
            break;
    }

    // Initialize local struct
    local = omf_calloc(1, sizeof(arena_local));
    scene_set_userdata(scene, local);

    // Set correct state
    local->state = ARENA_STATE_STARTING;
    local->ending_ticks = 0;
    local->rein_enabled = 0;

    local->round = 0;
    switch(scene->gs->match_settings.rounds) {
        case 0:
            local->rounds = 1;
            break;
        case 1:
            local->rounds = 3;
            break;
        case 2:
            local->rounds = 5;
            break;
        case 3:
            local->rounds = 7;
            break;
        default:
            local->rounds = 1;
            break;
    }

    local->tournament = false;
    local->over = 0;
    local->winner = 0;

    // TODO: Fire & Ice will need to set the arena palette
    unsigned pal_index = 0;
    if(scene->gs->init_flags->playback == 1) {
        // use palette index from rec
        pal_index = scene->gs->rec->arena_palette;
    } else if(scene->bk_data->file_id == 128 && is_tournament(scene->gs)) {
        // When playing the desert arena in Tournament mode,
        // pick a random palette to change the time of day.
        pal_index = rand_int(vector_size(&scene->bk_data->palettes));
    }

    // 0 is selected by default, so nothing to do if we hit that.
    if(pal_index > 0 && pal_index < vector_size(&scene->bk_data->palettes)) {
        vga_state_set_base_palette_from_range(bk_get_palette(scene->bk_data, pal_index), 0x60, 0x60, 0x40);
    }

    // Initial har data
    vec2i pos[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(HAR1_START_POS, ARENA_FLOOR);
    pos[1] = vec2i_create(HAR2_START_POS, ARENA_FLOOR);

    // init HARs
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(scene->gs, i);

        if(i == 0 && player->chr) {
            // we are in tournament mode
            local->rounds = 1;
            local->tournament = true;
        }
        object *obj = omf_calloc(1, sizeof(object));

        // load the player's colors into the palette
        palette_load_player_colors(&player->pilot->palette, i);

        // Create object and specialize it as HAR.
        // Errors are unlikely here, but check anyway.

        if(scene_load_har(scene, i)) {
            omf_free(obj);
            return 1;
        }

        object_create(obj, scene->gs, pos[i], vec2f_create(0, 0));
        if(har_create(obj, scene->af_data[i], dir[i], player->pilot->har_id, player->pilot->pilot_id, i)) {
            return 1;
        }

        // Enable HAR positional lighting if on Stadium arena (It's not used anywhere else).
        if(scene->bk_data->file_id == 8) {
            object_add_animation_effects(obj, EFFECT_POSITIONAL_LIGHTING);
        }

        // Set HAR to controller and game_player
        game_state_add_object(scene->gs, obj, RENDER_LAYER_MIDDLE, 0, 0);

        // Set HAR for player
        game_player_set_har(player, obj);
        game_player_get_ctrl(player)->har_obj_id = obj->id;

        if(player->pilot->photo) {
            // render pilot portraits
            object *portrait = omf_calloc(1, sizeof(object));
            if(i == 0) {
                object_create(portrait, scene->gs, vec2i_create(95, 0), vec2f_create(0, 0));
                sprite *sp = omf_calloc(1, sizeof(sprite));
                sprite_create(sp, player->pilot->photo, -1);
                portrait->x_percent = 0.70f;
                portrait->y_percent = 0.70f;
                object_set_sprite_override(portrait, 1);
                object_set_animation(portrait, create_animation_from_single(sp, vec2i_create(105, 0)));
                object_set_animation_owner(portrait, OWNER_OBJECT);
                portrait->cur_sprite_id = 0;
                game_state_add_object(scene->gs, portrait, RENDER_LAYER_TOP, 0, 0);
            } else {
                object_create(portrait, scene->gs, vec2i_create(225, 0), vec2f_create(0, 0));
                sprite *sp = omf_calloc(1, sizeof(sprite));
                sprite_create(sp, player->pilot->photo, -1);
                portrait->x_percent = 0.70f;
                portrait->y_percent = 0.70f;
                object_set_sprite_override(portrait, 1);
                object_set_animation(portrait, create_animation_from_single(sp, vec2i_create(235, 0)));
                object_set_direction(portrait, OBJECT_FACE_LEFT);
                object_set_animation_owner(portrait, OWNER_OBJECT);
                portrait->cur_sprite_id = 0;
                game_state_add_object(scene->gs, portrait, RENDER_LAYER_TOP, 0, 0);
            }

        } else if(local->rounds > 1) {
            // Create round tokens
            for(int j = 0; j < 4; j++) {
                if(j < ceilf(local->rounds / 2.0f)) {
                    object *round_token = omf_calloc(1, sizeof(object));
                    int xoff = 110 + 9 * j + 3 + j;
                    if(i == 1) {
                        xoff = 210 - 9 * j - 3 - j;
                    }
                    animation *ani = &bk_get_info(scene->bk_data, 27)->ani;
                    object_create(round_token, scene->gs, vec2i_create(xoff, 9), vec2f_create(0, 0));
                    local->player_rounds[i][j] = round_token->id;
                    object_set_animation(round_token, ani);
                    object_select_sprite(round_token, 1);
                    object_set_sprite_override(round_token, 1);
                    game_state_add_object(scene->gs, round_token, RENDER_LAYER_TOP, 0, 0);
                } else {
                    local->player_rounds[i][j] = 0;
                }
            }
        }
    }

    // remove the keyboard hooks

    game_player *_player[2];
    for(int i = 0; i < 2; i++) {
        _player[i] = game_state_get_player(scene->gs, i);
    }

    controller_set_repeat(game_player_get_ctrl(_player[0]), 1);
    controller_set_repeat(game_player_get_ctrl(_player[1]), 1);

    game_state_find_object(scene->gs, game_player_get_har_obj_id(_player[0]))->animation_state.enemy_obj_id =
        game_player_get_har_obj_id(_player[1]);
    game_state_find_object(scene->gs, game_player_get_har_obj_id(_player[1]))->animation_state.enemy_obj_id =
        game_player_get_har_obj_id(_player[0]);

    maybe_install_har_hooks(scene);

    // Arena menu theme
    gui_theme theme;
    gui_theme_defaults(&theme);
    theme.dialog.border_color = TEXT_MEDIUM_GREEN;
    theme.text.primary_color = TEXT_PRIMARY_COLOR;
    theme.text.secondary_color = TEXT_SECONDARY_COLOR;
    theme.text.disabled_color = TEXT_DISABLED_COLOR;
    theme.text.active_color = TEXT_ACTIVE_COLOR;
    theme.text.inactive_color = TEXT_INACTIVE_COLOR;
    theme.text.shadow_color = TEXT_SHADOW_COLOR;

    // Arena menu
    local->menu_visible = 0;
    local->game_menu = gui_frame_create(&theme, 60, 5, 181, 117);
    component *menu = menu_create();
    component *openomf = label_create_title("OPENOMF");
    label_set_text_color(openomf, TEXT_DARK_GREEN);
    menu_attach(menu, openomf);
    menu_attach(menu, filler_create());
    menu_attach(menu, filler_create());
    component *return_button =
        button_create("RETURN TO GAME", "Continue fighting.", false, false, game_menu_return, scene);
    widget_set_id(return_button, GAME_MENU_RETURN_ID);
    menu_attach(menu, return_button);

    menu_attach(menu, textslider_create_bind(
                          "SOUND", "Raise or lower the volume of all sound effects. Press left or right to change.", 10,
                          1, arena_sound_slide, NULL, &setting->sound.sound_vol));
    menu_attach(menu,
                textslider_create_bind("MUSIC", "Raise or lower the volume of music. Press right or left to change.",
                                       10, 1, arena_music_slide, NULL, &setting->sound.music_vol));

    component *speed_slider =
        textslider_create_bind("SPEED", "Change the speed of the game when in the arena. Press left or right to change",
                               10, 0, arena_speed_slide, scene, &setting->gameplay.speed);
    if(is_netplay(scene->gs)) {
        component_disable(speed_slider, 1);
    }
    menu_attach(menu, speed_slider);

    menu_attach(menu,
                button_create("VIDEO OPTIONS", "These are miscellaneous options for visual effects and detail levels.",
                              false, false, NULL, NULL));
    menu_attach(menu, button_create("HELP",
                                    "Obtain detailed and thorough explanation of the various options for which you "
                                    "may need a detailed and thorough explanation.",
                                    false, false, NULL, NULL));
    component *quit_button;
    if(is_tournament(scene->gs) && !scene->gs->match_settings.sim)
        quit_button = button_create("FORFEIT", lang_get(323), false, false, game_menu_quit, scene);
    else
        quit_button = button_create("QUIT", lang_get(322), false, false, game_menu_quit, scene);
    widget_set_id(quit_button, GAME_MENU_QUIT_ID);
    menu_attach(menu, quit_button);

    gui_frame_set_root(local->game_menu, menu);
    gui_frame_layout(local->game_menu);
    menu_select(menu, return_button);

    // Health and endurance bars
    local->health_bars[0] = progressbar_create(PROGRESSBAR_THEME_HEALTH, PROGRESSBAR_RIGHT, 100);
    component_layout(local->health_bars[0], 5, 5, 100, 8);
    local->health_bars[1] = progressbar_create(PROGRESSBAR_THEME_HEALTH, PROGRESSBAR_LEFT, 100);
    component_layout(local->health_bars[1], 215, 5, 100, 8);
    local->endurance_bars[0] = progressbar_create(PROGRESSBAR_THEME_ENDURANCE, PROGRESSBAR_RIGHT, 100);
    component_layout(local->endurance_bars[0], 5, 14, 100, 4);
    local->endurance_bars[1] = progressbar_create(PROGRESSBAR_THEME_ENDURANCE, PROGRESSBAR_LEFT, 100);
    component_layout(local->endurance_bars[1], 215, 14, 100, 4);

    // Score positioning
    chr_score_set_pos(game_player_get_score(_player[0]), 5, 33, OBJECT_FACE_RIGHT);
    chr_score_set_pos(game_player_get_score(_player[1]), 315, 33,
                      OBJECT_FACE_LEFT); // TODO: Set better coordinates for this

    // Possibly use tournament mode scoring
    chr_score_set_tournament_mode(game_player_get_score(_player[0]), local->tournament);
    chr_score_set_tournament_mode(game_player_get_score(_player[1]), local->tournament);

    // Reset the score
    chr_score_reset(game_player_get_score(_player[0]), !is_singleplayer(scene->gs));
    chr_score_reset(game_player_get_score(_player[1]), 1);

    // Reset the win counter in single player mode
    if(is_singleplayer(scene->gs)) {
        chr_score_reset_wins(game_player_get_score(_player[0]));
        chr_score_reset_wins(game_player_get_score(_player[1]));
    }

    // Reset screencaps
    har_screencaps_reset(&_player[0]->screencaps);
    har_screencaps_reset(&_player[1]->screencaps);

    // Set correct sounds for ready, round and number STL fields
    scene->bk_data->sound_translation_table[14] = 10;               // READY
    scene->bk_data->sound_translation_table[15] = 16;               // ROUND
    scene->bk_data->sound_translation_table[3] = 23 + local->round; // NUMBER

    // Disable the floating ball disappearence sound in fire arena
    if(scene->id == SCENE_ARENA3) {
        scene->bk_data->sound_translation_table[20] = 0;
    }

    if(local->rounds == 1) {
        // Start READY animation
        animation *ready_ani = &bk_get_info(scene->bk_data, 11)->ani;
        object *ready = omf_calloc(1, sizeof(object));
        object_create(ready, scene->gs, ready_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(ready, scene->bk_data->sound_translation_table);
        object_set_animation(ready, ready_ani);
        object_set_finish_cb(ready, scene_ready_anim_done);
        object_set_group(ready, GROUP_ANNOUNCEMENT);
        game_state_add_object(scene->gs, ready, RENDER_LAYER_TOP, 0, 0);
    } else {
        // ROUND
        animation *round_ani = &bk_get_info(scene->bk_data, 6)->ani;
        object *round = omf_calloc(1, sizeof(object));
        object_create(round, scene->gs, round_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(round, scene->bk_data->sound_translation_table);
        object_set_animation(round, round_ani);
        object_set_finish_cb(round, scene_ready_anim_done);
        object_set_group(round, GROUP_ANNOUNCEMENT);
        game_state_add_object(scene->gs, round, RENDER_LAYER_TOP, 0, 0);

        // Number
        animation *number_ani = &bk_get_info(scene->bk_data, 7)->ani;
        object *number = omf_calloc(1, sizeof(object));
        object_create(number, scene->gs, number_ani->start_pos, vec2f_create(0, 0));
        object_set_stl(number, scene->bk_data->sound_translation_table);
        object_set_animation(number, number_ani);
        object_select_sprite(number, local->round);
        object_set_group(number, GROUP_ANNOUNCEMENT);
        game_state_add_object(scene->gs, number, RENDER_LAYER_TOP, 0, 0);
    }

    // Callbacks
    scene_set_event_cb(scene, arena_event);
    scene_set_free_cb(scene, arena_free);
    scene_set_clone_free_cb(scene, arena_clone_free);
    scene_set_dynamic_tick_cb(scene, arena_dynamic_tick);
    scene_set_static_tick_cb(scene, arena_static_tick);
    scene_set_startup_cb(scene, arena_startup);
    scene_set_input_poll_cb(scene, arena_input_tick);
    scene_set_render_overlay_cb(scene, arena_render_overlay);
    scene_set_debug_cb(scene, arena_debug);
    scene->clone = arena_clone;

    // initialize recording, if we're not doing playback
    if(scene->gs->init_flags->playback == 0) {
        if(!scene->gs->rec) {
            scene->gs->rec = omf_calloc(1, sizeof(sd_rec_file));
        } else {
            // release previous recording
            sd_rec_free(scene->gs->rec);
        }
        sd_rec_create(scene->gs->rec);
        for(int i = 0; i < 2; i++) {
            // Declare some vars
            game_player *player = game_state_get_player(scene->gs, i);
            sd_pilot_clone(&scene->gs->rec->pilots[i].info, player->pilot);

            // this is the score when the REC started
            scene->gs->rec->scores[i] = game_player_get_score(player)->score;
        }
        scene->gs->rec->arena_id = scene->id - SCENE_ARENA0;
        scene->gs->rec->arena_palette = pal_index;
        scene->gs->rec->game_mode = is_tournament(scene->gs) ? REC_GAMEMODE_TOURNAMENT : REC_GAMEMODE_ARCADE;

        // player 1's controller
        switch(game_state_get_player(scene->gs, 0)->ctrl->type) {
            case CTRL_TYPE_KEYBOARD:
                // TODO figure out the actual keyboard type custom vs left vs right
                scene->gs->rec->p1_controller = REC_CONTROLLER_RIGHT_KEYBOARD;
                break;
            case CTRL_TYPE_GAMEPAD:
                scene->gs->rec->p1_controller = REC_CONTROLLER_JOYSTICK1;
                break;
            case CTRL_TYPE_AI:
                scene->gs->rec->p1_controller = REC_CONTROLLER_AI;
                break;
            case CTRL_TYPE_NETWORK:
                scene->gs->rec->p1_controller = REC_CONTROLLER_NETWORK;
                break;
            default:
                assert(false);
        }

        // player 2's controller
        switch(game_state_get_player(scene->gs, 1)->ctrl->type) {
            case CTRL_TYPE_KEYBOARD:
                // TODO figure out the actual keyboard type custom vs left vs right
                scene->gs->rec->p2_controller = REC_CONTROLLER_LEFT_KEYBOARD;
                break;
            case CTRL_TYPE_GAMEPAD:
                scene->gs->rec->p2_controller = REC_CONTROLLER_JOYSTICK2;
                break;
            case CTRL_TYPE_AI:
                scene->gs->rec->p2_controller = REC_CONTROLLER_AI;
                break;
            case CTRL_TYPE_NETWORK:
                scene->gs->rec->p2_controller = REC_CONTROLLER_NETWORK;
                break;
            default:
                assert(false);
        }

        // this is how p2 is actually configured
        switch(settings_get()->keys.ctrl_type1) {
            case CTRL_TYPE_KEYBOARD:
                scene->gs->rec->p2_controller_ = REC_CONTROLLER_LEFT_KEYBOARD;
                break;
            case CTRL_TYPE_GAMEPAD:
                scene->gs->rec->p2_controller_ = REC_CONTROLLER_JOYSTICK2;
                break;
            default:
                assert(false);
        }

        // capture the match settings
        scene->gs->rec->throw_range = scene->gs->match_settings.throw_range;
        scene->gs->rec->hit_pause = scene->gs->match_settings.hit_pause;
        scene->gs->rec->block_damage = scene->gs->match_settings.block_damage;
        scene->gs->rec->vitality = scene->gs->match_settings.vitality;
        scene->gs->rec->jump_height = scene->gs->match_settings.jump_height;
        scene->gs->rec->knock_down = scene->gs->match_settings.knock_down;
        scene->gs->rec->rehit_mode = scene->gs->match_settings.rehit;
        scene->gs->rec->def_throws = scene->gs->match_settings.defensive_throws;
        scene->gs->rec->power[0] = scene->gs->match_settings.power1;
        scene->gs->rec->power[1] = scene->gs->match_settings.power2;
        scene->gs->rec->hazards = scene->gs->match_settings.hazards;
        scene->gs->rec->round_type = scene->gs->match_settings.rounds;
        scene->gs->rec->hyper_mode = scene->gs->match_settings.fight_mode;
    }

    // All done!
    return 0;
}
