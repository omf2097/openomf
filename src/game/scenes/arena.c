#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <shadowdive/shadowdive.h>

#include "video/surface.h"
#include "video/video.h"
#include "game/scenes/arena.h"
#include "game/utils/progressbar.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "game/utils/settings.h"
#include "game/objects/har.h"
#include "game/objects/scrap.h"
#include "game/objects/hazard.h"
#include "game/protos/object.h"
#include "game/utils/score.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/utils/ticktimer.h"
#include "game/text/text.h"
#include "resources/languages.h"
#include "game/menu/menu.h"
#include "game/menu/menu_background.h"
#include "game/menu/textbutton.h"
#include "game/menu/textselector.h"
#include "game/menu/textslider.h"
#include "controller/controller.h"
#include "controller/net_controller.h"
#include "resources/ids.h"
#include "utils/log.h"
#include "utils/random.h"

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

#define HAR1_START_POS 110
#define HAR2_START_POS 211

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
    surface sur;
    int menu_visible;
    unsigned int state;
    int ending_ticks;

    progress_bar player1_health_bar;
    progress_bar player2_health_bar;
    progress_bar player1_endurance_bar;
    progress_bar player2_endurance_bar;

    chr_score player1_score;
    chr_score player2_score;

    int round;
    int rounds;
    int over;

    object *player_rounds[2][4];

    int rein_enabled;
} arena_local;

void arena_maybe_sync(scene *scene, int need_sync);

// -------- Local callbacks --------

void game_menu_quit(component *c, void *userdata) {
    scene *s = userdata;
    chr_score_reset(game_player_get_score(game_state_get_player((s)->gs, 0)), 1);
    chr_score_reset(game_player_get_score(game_state_get_player((s)->gs, 1)), 1);
    game_state_set_next(s->gs, SCENE_MENU);
}

void game_menu_return(component *c, void *userdata) {
    arena_local *local = scene_get_userdata((scene*)userdata);
    game_player *player1 = game_state_get_player(((scene*)userdata)->gs, 0);
    controller_set_repeat(game_player_get_ctrl(player1), 1);
    local->menu_visible = 0;
    game_state_set_paused(((scene*)userdata)->gs, 0);
    arena_maybe_sync(userdata, 1);
}

void music_slide(component *c, void *userdata, int pos) {
    music_set_volume(pos/10.0f);
}

void sound_slide(component *c, void *userdata, int pos) {
    sound_set_volume(pos/10.0f);
}

void arena_speed_slide(component *c, void *userdata, int pos) {
    scene *sc = userdata;
    game_state_set_speed(sc->gs, pos);
}

void scene_fight_anim_done(object *parent) {
    scene *scene = game_state_get_scene(parent->gs);
    arena_local *arena = scene_get_userdata(scene);

    // This will release HARs for action
    arena->state = ARENA_STATE_FIGHTING;

    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
}

void scene_fight_anim_start(void *userdata) {
    // Start FIGHT animation
    game_state *gs = userdata;
    scene *scene = game_state_get_scene(gs);
    animation *fight_ani = &bk_get_info(&scene->bk_data, 10)->ani;
    object *fight = malloc(sizeof(object));
    object_create(fight, gs, fight_ani->start_pos, vec2f_create(0,0));
    object_set_stl(fight, bk_get_stl(&scene->bk_data));
    object_set_animation(fight, fight_ani);
    object_set_finish_cb(fight, scene_fight_anim_done);
    game_state_add_object(gs, fight, RENDER_LAYER_TOP);
}

void scene_ready_anim_done(object *parent) {
    // Wait a moment before loading FIGHT animation
    ticktimer_add(&game_state_get_scene(parent->gs)->tick_timer, 10, scene_fight_anim_start, parent->gs);

    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
}

void scene_youwin_anim_done(object *parent) {
    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
}

void scene_youwin_anim_start(void *userdata) {
    // Start FIGHT animation
    game_state *gs = userdata;
    scene *scene = game_state_get_scene(gs);
    animation *youwin_ani = &bk_get_info(&scene->bk_data, 9)->ani;
    object *youwin = malloc(sizeof(object));
    object_create(youwin, gs, youwin_ani->start_pos, vec2f_create(0,0));
    object_set_stl(youwin, bk_get_stl(&scene->bk_data));
    object_set_animation(youwin, youwin_ani);
    object_set_finish_cb(youwin, scene_youwin_anim_done);
    game_state_add_object(gs, youwin, RENDER_LAYER_MIDDLE);

    // This will release HARs for action
    /*arena->state = ARENA_STATE_ENDING;*/
}

void scene_youlose_anim_done(object *parent) {
    // Custom object finisher callback requires that we
    // mark object as finished manually, if necessary.
    parent->animation_state.finished = 1;
}

void scene_youlose_anim_start(void *userdata) {
    // Start FIGHT animation
    game_state *gs = userdata;
    scene *scene = game_state_get_scene(gs);
    animation *youlose_ani = &bk_get_info(&scene->bk_data, 8)->ani;
    object *youlose = malloc(sizeof(object));
    object_create(youlose, gs, youlose_ani->start_pos, vec2f_create(0,0));
    object_set_stl(youlose, bk_get_stl(&scene->bk_data));
    object_set_animation(youlose, youlose_ani);
    object_set_finish_cb(youlose, scene_youlose_anim_done);
    game_state_add_object(gs, youlose, RENDER_LAYER_MIDDLE);

    // This will release HARs for action
    /*arena->state = ARENA_STATE_ENDING;*/
}

void arena_repeat_controller(void *userdata) {
    game_state *gs = userdata;
    game_player *player1 = game_state_get_player(gs, 0);
    controller_set_repeat(game_player_get_ctrl(player1), 1);
}

int is_netplay(scene *scene) {
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK ||
            game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
        return 1;
    }
    return 0;
}

int is_singleplayer(scene *scene) {
    if(game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI) {
        return 1;
    }
    return 0;
}

int is_demoplay(scene *scene) {
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_AI &&
       game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_AI) {
        return 1;
    }
    return 0;
}

int is_twoplayer(scene *scene) {
    if (!is_demoplay(scene) && !is_netplay(scene) && !is_singleplayer(scene)) {
        return 1;
    }
    return 0;
}

void arena_end(scene *sc) {
    game_state *gs = sc->gs;

    // XXX TODO take victory pose screenshot for the newsroom
    if (is_demoplay(sc)) {
        game_state_set_next(gs, rand_arena());
    }
    else if (is_singleplayer(sc)) {
        game_state_set_next(gs, SCENE_NEWSROOM);
    } else if (is_twoplayer(sc)) {
        game_state_set_next(gs, SCENE_MELEE);
    } else {
        game_state_set_next(gs, SCENE_MENU);
    }
}

void arena_reset(scene *sc) {
    arena_local *local = scene_get_userdata(sc);
    local->round++;
    local->state = ARENA_STATE_STARTING;

    // Kill all hazards and projectiles
    game_state_clear_hazards_projectiles(sc->gs);

    // Initial har data
    vec2i pos[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(HAR1_START_POS, 190);
    pos[1] = vec2i_create(HAR2_START_POS, 190);

    // init HARs
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(sc->gs, i);
        object *har_obj = game_player_get_har(player);
        har *h = object_get_userdata(har_obj);
        h->state = STATE_STANDING;
        har_set_ani(har_obj, ANIM_IDLE, 1);
        h->health = h->health_max;
        h->endurance = h->endurance_max;
        h->air_attacked = 0;
        object_set_pos(har_obj, pos[i]);
        object_set_vel(har_obj, vec2f_create(0, 0));
        object_set_gravity(har_obj, 1);
        object_set_direction(har_obj, dir[i]);
        chr_score_clear_done(&player->score);
    }

    sc->bk_data.sound_translation_table[3] = 23 + local->round; // NUMBER
    // ROUND animation
    animation *round_ani = &bk_get_info(&sc->bk_data, 6)->ani;
    object *round = malloc(sizeof(object));
    object_create(round, sc->gs, round_ani->start_pos, vec2f_create(0,0));
    object_set_stl(round, sc->bk_data.sound_translation_table);
    object_set_animation(round, round_ani);
    object_set_finish_cb(round, scene_ready_anim_done);
    game_state_add_object(sc->gs, round, RENDER_LAYER_TOP);

    // Round number
    animation *number_ani = &bk_get_info(&sc->bk_data, 7)->ani;
    object *number = malloc(sizeof(object));
    object_create(number, sc->gs, number_ani->start_pos, vec2f_create(0,0));
    object_set_stl(number, sc->bk_data.sound_translation_table);
    object_set_animation(number, number_ani);
    object_select_sprite(number, local->round);
    object_set_sprite_override(number, 1);
    game_state_add_object(sc->gs, number, RENDER_LAYER_TOP);
}

void arena_maybe_sync(scene *scene, int need_sync) {
    game_state *gs = scene->gs;
    game_player *player1 = game_state_get_player(gs, 0);
    game_player *player2 = game_state_get_player(gs, 1);

    if(need_sync
        && gs->role == ROLE_SERVER
        && (player1->ctrl->type == CTRL_TYPE_NETWORK || player2->ctrl->type == CTRL_TYPE_NETWORK)) {

        // some of the moves did something interesting and we should synchronize the peer
        serial ser;
        serial_create(&ser);
        game_state_serialize(scene->gs, &ser);
        if (player1->ctrl->type == CTRL_TYPE_NETWORK) {
            controller_update(player1->ctrl, &ser);
        }
        if (player2->ctrl->type == CTRL_TYPE_NETWORK) {
            controller_update(player2->ctrl, &ser);
        }
        serial_free(&ser);
    }
}

void arena_har_take_hit_hook(int hittee, af_move *move, scene *scene) {
    chr_score *score;
    chr_score *otherscore;
    object *hit_har;
    har *h;

    if (is_netplay(scene) && scene->gs->role == ROLE_CLIENT) {
        return; // netplay clients do not keep score
    }

    if (hittee == 1) {
        score = game_player_get_score(game_state_get_player(scene->gs, 0));
        otherscore = game_player_get_score(game_state_get_player(scene->gs, 1));
        hit_har = game_player_get_har(game_state_get_player(scene->gs, 1));
    } else {
        score = game_player_get_score(game_state_get_player(scene->gs, 1));
        otherscore = game_player_get_score(game_state_get_player(scene->gs, 0));
        hit_har = game_player_get_har(game_state_get_player(scene->gs, 0));
    }
    h = hit_har->userdata;
    if (h->state == STATE_RECOIL) {
        DEBUG("COMBO!");
    }
    chr_score_hit(score, move->points);
    chr_score_interrupt(otherscore, object_get_pos(hit_har));
    arena_maybe_sync(scene, 1);
}

void arena_har_recover_hook(int player_id, scene *scene) {
    chr_score *score;
    object *o_har;

    if (is_netplay(scene) && scene->gs->role == ROLE_CLIENT) {
        return; // netplay clients do not keep score
    }

    if (player_id == 0) {
        score = game_player_get_score(game_state_get_player(scene->gs, 1));
        o_har = game_player_get_har(game_state_get_player(scene->gs, 1));
    } else {
        score = game_player_get_score(game_state_get_player(scene->gs, 0));
        o_har = game_player_get_har(game_state_get_player(scene->gs, 0));
    }
    if(chr_score_end_combo(score, object_get_pos(o_har))) {
        arena_maybe_sync(scene, 1);
    }
}

void arena_har_hit_wall_hook(int player_id, int wall, scene *scene) {
    object *o_har = game_player_get_har(game_state_get_player(scene->gs, player_id));
    har *h = object_get_userdata(o_har);
    if (scene->id == SCENE_ARENA2 && o_har->pos.y < 190 && (h->state == STATE_FALLEN || h->state == STATE_RECOIL) && abs(o_har->vel.x) >= 1) {
        bk_info *info = bk_get_info(&scene->bk_data, 20+wall);
        object *obj = malloc(sizeof(object));
        object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0,0));
        object_set_stl(obj, scene->bk_data.sound_translation_table);
        object_set_animation(obj, &info->ani);
        obj->singleton = 1;
        if (game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM) == 0) {
            h->state = STATE_FALLEN;
            // Set hit animation
            object_set_animation(o_har, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
            object_set_repeat(o_har, 0);
            // from MASTER.DAT
            object_set_custom_string(o_har, "hQ1-hQ7-x-3Q5-x-2L5-x-2M900");
            scene->gs->screen_shake_horizontal = 3*fabsf(o_har->vel.x);
            if(scene->gs->screen_shake_horizontal == 0) {
                scene->gs->screen_shake_horizontal = 16;
            }
            object_set_vel(obj, vec2f_create(0, 0));

            // spawn the electricity on top of the HAR
            // TODO this doesn't track the har's position well...
            info = bk_get_info(&scene->bk_data, 22);
            object *obj2 = malloc(sizeof(object));
            object_create(obj2, scene->gs, vec2i_create(o_har->pos.x, o_har->pos.y), vec2f_create(0, 0));
            object_set_stl(obj2, scene->bk_data.sound_translation_table);
            object_set_animation(obj2, &info->ani);
            object_dynamic_tick(obj2);
            game_state_add_object(scene->gs, obj2, RENDER_LAYER_TOP);
        } else {
            object_free(obj);
            free(obj);
        }
        return;
    }

    if (scene->id == SCENE_ARENA4 && o_har->pos.y < 190) {
        DEBUG("hit desert wall %d", wall);
        // desert always shows the 'hit' animation when you touch the wall
        bk_info *info = bk_get_info(&scene->bk_data, 20+wall);
        object *obj = malloc(sizeof(object));
        object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0,0));
        object_set_stl(obj, scene->bk_data.sound_translation_table);
        object_set_animation(obj, &info->ani);
        object_set_custom_string(obj, "brwA1-brwB1-brwD1-brwE0-brwD4-brwC2-brwB2-brwA2");
        obj->singleton = 1;
        if(game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM) != 0) {
            object_free(obj);
            free(obj);
        }
    }
#ifdef DEBUGMODE_STFU
    DEBUG("velocity %d", abs(o_har->vel.x));
#endif
    if ((h->state == STATE_FALLEN || h->state == STATE_RECOIL) && abs(o_har->vel.x) > 5) {
        h->state = STATE_RECOIL;
        // Set hit animation
        object_set_animation(o_har, &af_get_move(h->af_data, ANIM_DAMAGE)->ani);
        object_set_repeat(o_har, 0);
        scene->gs->screen_shake_horizontal = 3*fabsf(o_har->vel.x);
        // from MASTER.DAT
        object_set_custom_string(o_har, "hQ10-x-3Q5-x-2L5-x-2M900");
        o_har->vel.x = 0.f;
    }
}

void arena_har_defeat_hook(int player_id, scene *scene) {
    game_state *gs = scene->gs;
    arena_local *local = scene_get_userdata(scene);
    int other_player_id = abs(player_id - 1);
    game_player *player_winner = game_state_get_player(scene->gs, other_player_id);
    game_player *player_loser = game_state_get_player(scene->gs, player_id);
    object *winner  = game_player_get_har(player_winner);
    object *loser   = game_player_get_har(player_loser);
    har *winner_har = object_get_userdata(winner);
    // XXX need a smarter way to detect if a player is networked or local
    if(player_winner->ctrl->type != CTRL_TYPE_NETWORK &&
            player_loser->ctrl->type == CTRL_TYPE_NETWORK) {
        scene_youwin_anim_start(scene->gs);
    } else if(player_winner->ctrl->type == CTRL_TYPE_NETWORK &&
            player_loser->ctrl->type != CTRL_TYPE_NETWORK) {
        scene_youlose_anim_start(scene->gs);
    } else {
        if (!is_singleplayer(scene)) {
            // XXX in two player mode, "you win" should always be displayed
            scene_youwin_anim_start(scene->gs);
        } else {
            if (player_id == 1) {
                scene_youwin_anim_start(scene->gs);
            } else {
                scene_youlose_anim_start(scene->gs);
            }
        }
    }
    chr_score *score = game_player_get_score(game_state_get_player(gs, other_player_id));
    object_select_sprite(local->player_rounds[other_player_id][score->rounds], 0);
    score->rounds++;
    if (score->rounds >= ceil(local->rounds/2.0f)) {
        har_set_ani(winner, ANIM_VICTORY, 0);
        chr_score_victory(score, winner_har->health);
        winner_har->state = STATE_VICTORY;
        local->over = 1;
        if (is_singleplayer(scene)) {
          player_winner->sp_wins |= 2 << player_loser->pilot_id;
          if (player_loser->pilot_id == 10) {
              // can't scrap/destruct kreissack
              winner_har->state = STATE_DONE;
              // major go boom
              har_set_ani(loser, 47, 1);
          }
        }
    } else {
        har_set_ani(winner, ANIM_VICTORY, 0);
        // can't do scrap/destruct except on final round
        winner_har->state = STATE_DONE;
    }
    winner_har->executing_move = 1;
    object_set_vel(loser, vec2f_create(0, 0));
    object_set_vel(winner, vec2f_create(0, 0));
    //object_set_gravity(loser, 0);
    arena_maybe_sync(scene,
            chr_score_interrupt(score, object_get_pos(winner)));
}

void arena_maybe_turn_har(int player_id, scene* scene) {
    int other_player_id = abs(player_id - 1);
    object *obj_har1 = game_player_get_har(game_state_get_player(scene->gs, player_id));
    object *obj_har2 = game_player_get_har(game_state_get_player(scene->gs, other_player_id));
    if (obj_har1->pos.x > obj_har2->pos.x) {
        object_set_direction(obj_har1, OBJECT_FACE_LEFT);
    } else {
        object_set_direction(obj_har1, OBJECT_FACE_RIGHT);
    }

    // there isn;t an idle event hook, so do the best we can...
    har *har2 = obj_har2->userdata;
    if ((har2->state == STATE_STANDING || har_is_crouching(har2) || har_is_walking(har2)) && !har2->executing_move) {
        object_set_direction(obj_har2, object_get_direction(obj_har1) * -1);
    }
}

void arena_har_hook(har_event event, void *data) {
    scene *scene = data;
    int other_player_id = abs(event.player_id - 1);
    arena_local *arena = scene_get_userdata(scene);
    chr_score *score = game_player_get_score(game_state_get_player(scene->gs, event.player_id));
    object *obj_har1 = game_player_get_har(game_state_get_player(scene->gs, event.player_id));
    object *obj_har2 = game_player_get_har(game_state_get_player(scene->gs, other_player_id));
    har *har1 = obj_har1->userdata;
    har *har2 = obj_har2->userdata;
    switch (event.type) {
        case HAR_EVENT_WALK:
            arena_maybe_turn_har(event.player_id, scene);
            break;
        case HAR_EVENT_AIR_TURN:
            arena_maybe_turn_har(event.player_id, scene);
            break;
        case HAR_EVENT_TAKE_HIT:
            if(af_get_move(har2->af_data, obj_har2->cur_animation->id)->category != CAT_CLOSE) {
                arena_maybe_turn_har(event.player_id, scene);
            }
            arena_har_take_hit_hook(event.player_id, event.move, scene);
            break;
        case HAR_EVENT_HIT_WALL:
            arena_har_hit_wall_hook(event.player_id, event.wall, scene);
            break;
        case HAR_EVENT_ATTACK:
            if(object_is_airborne(obj_har1)) {
                har1->air_attacked = 1;
                DEBUG("AIR ATTACK %u", event.player_id);
            } else {
                arena_maybe_turn_har(event.player_id, scene);
            }
            break;
        case HAR_EVENT_LAND:
            arena_maybe_turn_har(event.player_id, scene);
            har1->air_attacked = 0;
            DEBUG("LAND %u", event.player_id);
            arena_maybe_sync(scene, 1);
            break;
        case HAR_EVENT_RECOVER:
            arena_har_recover_hook(event.player_id, scene);
            if(!object_is_airborne(obj_har1)) {
                arena_maybe_turn_har(event.player_id, scene);
                har1->air_attacked = 0;
                DEBUG("RECOVER %u", event.player_id);
            }
            break;
        case HAR_EVENT_DEFEAT:
            arena_har_defeat_hook(event.player_id, scene);
            if (arena->state != ARENA_STATE_ENDING) {
                arena->ending_ticks = 0;
                arena->state = ARENA_STATE_ENDING;
            }
            break;
        case HAR_EVENT_SCRAP:
            chr_score_scrap(score);
            break;
        case HAR_EVENT_DESTRUCTION:
            chr_score_destruction(score);
            DEBUG("DESTRUCTION!");
            break;
        case HAR_EVENT_DONE:
            chr_score_done(score);
            DEBUG("DONE!");
            break;
    }
}

void maybe_install_har_hooks(scene *scene) {
    object *obj_har1,*obj_har2;
    obj_har1 = game_player_get_har(game_state_get_player(scene->gs, 0));
    obj_har2 = game_player_get_har(game_state_get_player(scene->gs, 1));
    har *har1, *har2;
    har1 = obj_har1->userdata;
    har2 = obj_har2->userdata;

    if (scene->gs->role == ROLE_CLIENT) {
        game_player *_player[2];
        for(int i = 0; i < 2; i++) {
            _player[i] = game_state_get_player(scene->gs, i);
        }
        if(game_player_get_ctrl(_player[0])->type == CTRL_TYPE_NETWORK) {
            har_install_action_hook(har2, &net_controller_har_hook, _player[0]->ctrl);
        }
        if(game_player_get_ctrl(_player[1])->type == CTRL_TYPE_NETWORK) {
            har_install_action_hook(har1, &net_controller_har_hook, _player[1]->ctrl);
        }
    }

    har_install_hook(har1, &arena_har_hook, scene);
    har_install_hook(har2, &arena_har_hook, scene);
}


// -------- Scene callbacks --------

void arena_free(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    game_state_set_paused(scene->gs, 0);

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_har(player, NULL);
        //game_player_set_ctrl(player, NULL);
        controller_set_repeat(game_player_get_ctrl(player), 0);

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

    surface_free(&local->sur);

    music_stop();

    progressbar_free(&local->player1_health_bar);
    progressbar_free(&local->player2_health_bar);
    progressbar_free(&local->player1_endurance_bar);
    progressbar_free(&local->player2_endurance_bar);

    settings_save();

    free(local);
}

int arena_handle_events(scene *scene, game_player *player, ctrl_event *i) {
    int need_sync = 0;
    arena_local *local = scene_get_userdata(scene);
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION && i->event_data.action == ACT_ESC && player == game_state_get_player(scene->gs, 0)) {
                // toggle menu
                local->menu_visible = !local->menu_visible;
                game_state_set_paused(scene->gs, local->menu_visible);
                need_sync = 1;
                controller_set_repeat(game_player_get_ctrl(player), !local->menu_visible);
                DEBUG("local menu %d, controller repeat %d", local->menu_visible, game_player_get_ctrl(player)->repeat);
            } else if(i->type == EVENT_TYPE_ACTION && local->menu_visible && player == game_state_get_player(scene->gs, 0) && !is_demoplay(scene)) {
                DEBUG("menu event %d", i->event_data.action);
                // menu events
                menu_handle_action(&local->game_menu, i->event_data.action);
            } else if(i->type == EVENT_TYPE_ACTION) {
                if (player->ctrl->type == CTRL_TYPE_NETWORK) {
                    do {
                        object_act(game_player_get_har(player), i->event_data.action);
                    } while ((i = i->next) && i->type == EVENT_TYPE_ACTION);
                    // always trigger a synchronization, since if the client's move did not actually happen, we want to rewind them ASAP
                    need_sync = 1;
                    // XXX do we need to continue here, since we screwed with 'i'?
                } else {
                    need_sync += object_act(game_player_get_har(player), i->event_data.action);
                }
            } else if (i->type == EVENT_TYPE_SYNC) {
                DEBUG("sync");
                game_state_unserialize(scene->gs, i->event_data.ser, player->ctrl->rtt);
                maybe_install_har_hooks(scene);
            } else if (i->type == EVENT_TYPE_CLOSE) {
                game_state_set_next(scene->gs, SCENE_MENU);
                return 0;
            }
        } while(i && (i = i->next));
    }
    return need_sync;
}

void arena_spawn_hazard(scene *scene) {
    iterator it;
    hashmap_iter_begin(&scene->bk_data.infos, &it);
    hashmap_pair *pair = NULL;

    if (is_netplay(scene) && scene->gs->role == ROLE_CLIENT) {
        // only the server spawns hazards
        return;
    }

    int changed = 0;

    while((pair = iter_next(&it)) != NULL) {
        bk_info *info = (bk_info*)pair->val;
        if(info->probability > 1) {
            if (rand_int(info->probability) == 1) {
                // TODO don't spawn it if we already have this animation running
                object *obj = malloc(sizeof(object));
                object_create(obj, scene->gs, info->ani.start_pos, vec2f_create(0,0));
                object_set_stl(obj, scene->bk_data.sound_translation_table);
                object_set_animation(obj, &info->ani);
                if (scene->id == SCENE_ARENA3 && info->ani.id == 0) {
                    // XXX fire pit orb has a bug whwre it double spawns. Use a custom animation string to avoid it
                    // it mioght be to do with the 'mp' tag, which we don't currently understand
                    object_set_custom_string(obj, "Z3-mx+160my+100m15mp10Z1-Z300");
                }
                /*object_set_spawn_cb(obj, cb_scene_spawn_object, (void*)scene);*/
                /*object_set_destroy_cb(obj, cb_scene_destroy_object, (void*)scene);*/
                hazard_create(obj, scene);
                obj->singleton = 1;
                if (game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM) == 0) {
                    object_set_layers(obj, LAYER_HAZARD|LAYER_HAR);
                    object_set_group(obj, GROUP_PROJECTILE);
                    object_set_userdata(obj, &scene->bk_data);
                    if (info->ani.extra_string_count > 0) {
                        // For the desert, there's a bunch of extra animation strgins for
                        // the different plane formations.
                        // Pick one, rather than always use the first

                        int r = rand_int(info->ani.extra_string_count);
                        if (r > 0) {
                            str *s = vector_get(&info->ani.extra_strings, r);
                            object_set_custom_string(obj, str_c(s));
                        }
                    }

                    // XXX without this, the object does not unserialize correctly in netplay
                    object_dynamic_tick(obj);

                    DEBUG("Arena tick: Hazard with probability %d started.", info->probability, info->ani.id);
                    changed++;
                } else {
                    object_free(obj);
                    free(obj);
                }
            }
        }
    }

    arena_maybe_sync(scene, changed);
}

void arena_dynamic_tick(scene *scene, int paused) {
    arena_local *local = scene_get_userdata(scene);
    game_state *gs = scene->gs;
    game_player *player1 = game_state_get_player(gs, 0);
    game_player *player2 = game_state_get_player(gs, 1);

    if(!paused) {
        // Handle scrolling score texts
        chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 0)));
        chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 1)));

        // Turn the HARs to face the enemy
        object *obj_har1,*obj_har2;
        obj_har1 = game_player_get_har(game_state_get_player(scene->gs, 0));
        obj_har2 = game_player_get_har(game_state_get_player(scene->gs, 1));
        har *har1, *har2;
        har1 = obj_har1->userdata;
        har2 = obj_har2->userdata;

        har1->delay = ceil(player2->ctrl->rtt / 2.0f);
        har2->delay = ceil(player1->ctrl->rtt / 2.0f);

        if(local->state != ARENA_STATE_ENDING && local->state != ARENA_STATE_STARTING) {
            settings *setting = settings_get();
            if (setting->gameplay.hazards_on) {
                arena_spawn_hazard(scene);
            }
        }
        if(local->state == ARENA_STATE_ENDING) {
            chr_score *s1 = game_player_get_score(game_state_get_player(scene->gs, 0));
            chr_score *s2 = game_player_get_score(game_state_get_player(scene->gs, 1));
            if (player_frame_isset(obj_har1, "be")
                || player_frame_isset(obj_har2, "be")
                || chr_score_onscreen(s1)
                || chr_score_onscreen(s2)) {
                /*DEBUG("blocking ending");*/
            } else {
                local->ending_ticks++;
            }
            if(local->ending_ticks > 20) {
                if (!local->over) {
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
                for(int harnum = 0;harnum < game_state_num_players(gs);harnum++) {
                    object *h_obj = game_state_get_player(gs, harnum)->har;
                    har *h = object_get_userdata(h_obj);
                    // Calculate velocity etc.
                    float rv = rand_float() - 0.5f;
                    float velx = rv;
                    float vely = -12 * sin(0 / 2 + rv);

                    // Make sure scrap has somekind of velocity
                    // (to prevent floating scrap objects)
                    if(vely < 0.1 && vely > -0.1) vely += 0.21;

                    // Create the object
                    object *scrap = malloc(sizeof(object));
                    int anim_no = rand_int(3) + ANIM_SCRAP_METAL;
                    object_create(scrap, gs, pos, vec2f_create(velx, vely));
                    object_set_animation(scrap, &af_get_move(h->af_data, anim_no)->ani);
                    object_set_gravity(scrap, 0.4f);
                    object_set_pal_offset(scrap, object_get_pal_offset(h_obj));
                    object_set_layers(scrap, LAYER_SCRAP);
                    object_dynamic_tick(scrap);
                    scrap->cast_shadow = 1;
                    scrap_create(scrap);
                    game_state_add_object(gs, scrap, RENDER_LAYER_TOP);
                }
            }
        }
    } // if(!paused)

    int need_sync = 0;
    // allow enemy HARs to move during a network game
    need_sync += arena_handle_events(scene, player1, player1->ctrl->extra_events);
    need_sync += arena_handle_events(scene, player2, player2->ctrl->extra_events);
    arena_maybe_sync(scene, need_sync);
}

void arena_static_tick(scene *scene, int paused) {
    arena_local *local = scene_get_userdata(scene);
    menu_tick(&local->game_menu);
}

void arena_input_tick(scene *scene) {
    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    ctrl_event *p1 = NULL, *p2 = NULL;
    controller_poll(player1->ctrl, &p1);
    controller_poll(player2->ctrl, &p2);

    int need_sync = 0;
    need_sync += arena_handle_events(scene, player1, p1);
    need_sync += arena_handle_events(scene, player2, p2);
    controller_free_chain(p1);
    controller_free_chain(p2);
    arena_maybe_sync(scene, need_sync);
}

int arena_event(scene *scene, SDL_Event *e) {
    // ESC during demo mode jumps you back to the main menu
    if (e->type == SDL_KEYDOWN && is_demoplay(scene) && e->key.keysym.sym == SDLK_ESCAPE) {
        game_state_set_next(scene->gs, SCENE_MENU);
    }
    return 0;
}

void arena_render_overlay(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    // Render bars
    game_player *player[2];
    object *obj[2];

    char buf[40];
#ifdef DEBUGMODE
    sprintf(buf, "%u", game_state_get_tick(scene->gs));
    font_render(&font_small, buf, 160, 0, TEXT_COLOR);
    sprintf(buf, "%u", rand_get_seed());
    font_render(&font_small, buf, 130, 8, TEXT_COLOR);
#endif
    har *har[2];
    for(int i = 0; i < 2; i++) {
        player[i] = game_state_get_player(scene->gs, i);
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

        if (p1_en * 100 < 50 && scene->gs->tick % 8 < 4) {
            progressbar_render_flashing(&local->player1_endurance_bar, 1);
        } else {
            progressbar_render(&local->player1_endurance_bar);
        }

        if (p2_en * 100 < 50 && scene->gs->tick % 8 < 4) {
            progressbar_render_flashing(&local->player2_endurance_bar, 1);
        } else {
            progressbar_render(&local->player2_endurance_bar);
        }

        // Render HAR and pilot names
        font_render_shadowed(&font_small,
                            lang_get(player[0]->pilot_id+20),
                            5, 19,
                            TEXT_COLOR,
                            TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        font_render_shadowed(&font_small,
                            lang_get((player[0]->har_id - HAR_JAGUAR)+31),
                            5, 26,
                            TEXT_COLOR,
                            TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);

        int p2len = (strlen(lang_get(player[1]->pilot_id+20))-1) * font_small.w;
        int h2len = (strlen(lang_get((player[1]->har_id - HAR_JAGUAR)+31))-1) * font_small.w;
        font_render_shadowed(&font_small,
                            lang_get(player[1]->pilot_id+20),
                            315-p2len, 19,
                            TEXT_COLOR,
                            TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);
        font_render_shadowed(&font_small,
                            lang_get((player[1]->har_id - HAR_JAGUAR)+31),
                            315-h2len, 26,
                            TEXT_COLOR,
                            TEXT_SHADOW_RIGHT|TEXT_SHADOW_BOTTOM);

        // Render score stuff
        chr_score_render(game_player_get_score(player[0]));

        // Do not render player 2 score in 1 player mode
        if(game_player_get_selectable(player[1])) {
            chr_score_render(game_player_get_score(player[1]));
        }

        // render ping, if player is networked
        if (player[0]->ctrl->type == CTRL_TYPE_NETWORK) {
            sprintf(buf, "ping %u", player[0]->ctrl->rtt);
            font_render(&font_small, buf, 5, 40, TEXT_COLOR);
        }
        if (player[1]->ctrl->type == CTRL_TYPE_NETWORK) {
            sprintf(buf, "ping %u", player[1]->ctrl->rtt);
            font_render(&font_small, buf, 315-(strlen(buf)*font_small.w), 40, TEXT_COLOR);
        }

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 4; j++) {
                if (local->player_rounds[i][j]) {
                    object_render(local->player_rounds[i][j]);
                }
            }
        }
    }

    // Render menu (if visible)
    if(local->menu_visible) {
        menu_render(&local->game_menu);
        video_render_sprite(&local->sur, 10, 150, BLEND_ALPHA, 0);
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

int arena_create(scene *scene) {
    settings *setting;
    arena_local *local;

    // Load up settings
    setting = settings_get();

    // Initialize Demo
    if(is_demoplay(scene)) {
        game_state_init_demo(scene->gs);
    }

    // Handle music playback
    music_stop();
    char *music_filename = NULL;
    switch(scene->bk_data.file_id) {
        case 8:   music_filename = get_path_by_id(PSM_ARENA0); break;
        case 16:  music_filename = get_path_by_id(PSM_ARENA1); break;
        case 32:  music_filename = get_path_by_id(PSM_ARENA2); break;
        case 64:  music_filename = get_path_by_id(PSM_ARENA3); break;
        case 128: music_filename = get_path_by_id(PSM_ARENA4); break;
    }
    if(music_filename != NULL) {
        music_play(music_filename);
        music_set_volume(setting->sound.music_vol/10.0f);
        free(music_filename);
    }

    // Initialize local struct
    local = malloc(sizeof(arena_local));
    scene_set_userdata(scene, local);

    // Set correct state
    local->state = ARENA_STATE_STARTING;
    local->ending_ticks = 0;
    local->rein_enabled = 0;

    local->round = 0;
    switch (setting->gameplay.rounds) {
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
    local->over = 0;

    // Initial har data
    vec2i pos[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(HAR1_START_POS, 190);
    pos[1] = vec2i_create(HAR2_START_POS, 190);

    // init HARs
    for(int i = 0; i < 2; i++) {
        // Declare some vars
        game_player *player = game_state_get_player(scene->gs, i);
        object *obj = malloc(sizeof(object));

        // load the player's colors into the palette
        palette *base_pal = video_get_base_palette();
        palette_set_player_color(base_pal, i, player->colors[2], 0);
        palette_set_player_color(base_pal, i, player->colors[1], 1);
        palette_set_player_color(base_pal, i, player->colors[0], 2);
        video_force_pal_refresh();

        // Create object and specialize it as HAR.
        // Errors are unlikely here, but check anyway.

        if (scene_load_har(scene, i, player->har_id)) {
            free(obj);
            return 1;
        }

        object_create(obj, scene->gs, pos[i], vec2f_create(0,0));
        if(har_create(obj, scene->af_data[i], dir[i], player->har_id, player->pilot_id, i)) {
            return 1;
        }

        // Set HAR to controller and game_player
        game_state_add_object(scene->gs, obj, RENDER_LAYER_MIDDLE);

        // Set HAR for player
        game_player_set_har(player, obj);
        game_player_get_ctrl(player)->har = obj;

        // Create round tokens
        for (int j = 0; j < 4; j++) {
            if (j < ceil(local->rounds / 2.0f)) {
                local->player_rounds[i][j] = malloc(sizeof(object));
                int xoff = 110 + 9 * j + 3 + j;
                if (i == 1) {
                    xoff = 210 - 9 * j - 3 - j;
                }
                animation *ani = &bk_get_info(&scene->bk_data, 27)->ani;
                object_create(local->player_rounds[i][j], scene->gs, vec2i_create(xoff ,9), vec2f_create(0, 0));
                object_set_animation(local->player_rounds[i][j], ani);
                object_select_sprite(local->player_rounds[i][j], 1);
            } else {
                local->player_rounds[i][j] = NULL;
            }
        }
    }

    // remove the keyboard hooks

    game_player *_player[2];
    for(int i = 0; i < 2; i++) {
        _player[i] = game_state_get_player(scene->gs, i);
    }
    if(game_player_get_ctrl(_player[0])->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(game_player_get_ctrl(_player[1]));
    }
    if(game_player_get_ctrl(_player[1])->type == CTRL_TYPE_NETWORK) {
        controller_clear_hooks(game_player_get_ctrl(_player[0]));
    }

    controller_set_repeat(game_player_get_ctrl(_player[0]), 1);
    controller_set_repeat(game_player_get_ctrl(_player[1]), 1);

    game_player_get_har(_player[0])->animation_state.enemy = game_player_get_har(_player[1]);
    game_player_get_har(_player[1])->animation_state.enemy = game_player_get_har(_player[0]);

    maybe_install_har_hooks(scene);

    // Arena menu
    local->menu_visible = 0;
    menu_create(&local->game_menu, 70, 5, 181, 117);
    textbutton_create(&local->title_button, &font_large, "OMF 2097");
    textbutton_create(&local->return_button, &font_large, "RETURN TO GAME");
    textslider_create(&local->sound_slider, &font_large, "SOUND", 10, 1);
    textslider_create(&local->music_slider, &font_large, "MUSIC", 10, 1);
    textslider_create(&local->speed_slider, &font_large, "SPEED", 10, 0);
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
    local->speed_slider.userdata = (void*)scene;
    local->speed_slider.slide = arena_speed_slide;
    if (is_netplay(scene)) {
        // no changing the speed during netplay
        local->speed_slider.disabled = 1;
    }

    local->title_button.disabled = 1;

    // Events
    local->quit_button.userdata = (void*)scene;
    local->quit_button.click = game_menu_quit;
    local->return_button.userdata = (void*)scene;
    local->return_button.click = game_menu_return;

    menu_select(&local->game_menu, &local->return_button);

    // background for the 'help' at the bottom of the screen
    // TODO support rendering text onto it
    menu_background_create(&local->sur, 301, 37);

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
    progressbar_create_flashing(&local->player1_endurance_bar,
                       5, 14, 100, 4,
                       BAR_COLOR_TL_BORDER,
                       BAR_COLOR_BR_BORDER,
                       BAR_COLOR_BG,
                       BAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_RIGHT);
    progressbar_create_flashing(&local->player2_endurance_bar,
                       215, 14, 100, 4,
                       BAR_COLOR_TL_BORDER,
                       BAR_COLOR_BR_BORDER,
                       BAR_COLOR_BG,
                       BAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_TL_BORDER,
                       ENDURANCEBAR_COLOR_BR_BORDER,
                       ENDURANCEBAR_COLOR_BG,
                       PROGRESSBAR_LEFT);
    chr_score_set_pos(game_player_get_score(_player[0]), 5, 33, OBJECT_FACE_RIGHT);
    chr_score_set_pos(game_player_get_score(_player[1]), 315, 33, OBJECT_FACE_LEFT); // TODO: Set better coordinates for this

    // Reset the score
    chr_score_reset(game_player_get_score(_player[0]), !is_singleplayer(scene));
    chr_score_reset(game_player_get_score(_player[1]), 1);

    // Reset the win counter in single player mode
    if(is_singleplayer(scene)) {
        chr_score_reset_wins(game_player_get_score(_player[0]));
        chr_score_reset_wins(game_player_get_score(_player[1]));
    }

    // TODO: Do something about this hack!
    scene->bk_data.sound_translation_table[14] = 10; // READY
    scene->bk_data.sound_translation_table[15] = 16; // ROUND
    scene->bk_data.sound_translation_table[3] = 23 + local->round; // NUMBER

    if (local->rounds == 1) {
        // Start READY animation
        animation *ready_ani = &bk_get_info(&scene->bk_data, 11)->ani;
        object *ready = malloc(sizeof(object));
        object_create(ready, scene->gs, ready_ani->start_pos, vec2f_create(0,0));
        object_set_stl(ready, scene->bk_data.sound_translation_table);
        object_set_animation(ready, ready_ani);
        object_set_finish_cb(ready, scene_ready_anim_done);
        game_state_add_object(scene->gs, ready, RENDER_LAYER_TOP);
    } else {
        // ROUND
        animation *round_ani = &bk_get_info(&scene->bk_data, 6)->ani;
        object *round = malloc(sizeof(object));
        object_create(round, scene->gs, round_ani->start_pos, vec2f_create(0,0));
        object_set_stl(round, scene->bk_data.sound_translation_table);
        object_set_animation(round, round_ani);
        object_set_finish_cb(round, scene_ready_anim_done);
        game_state_add_object(scene->gs, round, RENDER_LAYER_TOP);

        // Number
        animation *number_ani = &bk_get_info(&scene->bk_data, 7)->ani;
        object *number = malloc(sizeof(object));
        object_create(number, scene->gs, number_ani->start_pos, vec2f_create(0,0));
        object_set_stl(number, scene->bk_data.sound_translation_table);
        object_set_animation(number, number_ani);
        object_select_sprite(number, local->round);
        game_state_add_object(scene->gs, number, RENDER_LAYER_TOP);
    }

    // Callbacks
    scene_set_event_cb(scene, arena_event);
    scene_set_free_cb(scene, arena_free);
    scene_set_dynamic_tick_cb(scene, arena_dynamic_tick);
    scene_set_static_tick_cb(scene, arena_static_tick);
    scene_set_input_poll_cb(scene, arena_input_tick);
    scene_set_render_overlay_cb(scene, arena_render_overlay);

    // Pick renderer
    video_select_renderer(VIDEO_RENDERER_HW);

    // All done!
    return 0;
}
