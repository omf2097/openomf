#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <shadowdive/shadowdive.h>

#include "video/surface.h"
#include "video/video.h"
#include "game/scenes/arena.h"
#include "game/scenes/progressbar.h"
#include "audio/stream.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "game/settings.h"
#include "game/objects/har.h"
#include "game/objects/scrap.h"
#include "game/objects/hazard.h"
#include "game/protos/object.h"
#include "game/score.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/ticktimer.h"
#include "game/text/text.h"
#include "game/text/languages.h"
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

    int rein_enabled;
} arena_local;

// -------- Local callbacks --------

void game_menu_quit(component *c, void *userdata) {
    scene *s = userdata;
    game_state_set_next(s->gs, SCENE_MENU);
}

void game_menu_return(component *c, void *userdata) {
    arena_local *local = scene_get_userdata((scene*)userdata);
    local->menu_visible = 0;
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
    arena_local *arena = scene_get_userdata(scene);
    animation *youwin_ani = &bk_get_info(&scene->bk_data, 9)->ani;
    object *youwin = malloc(sizeof(object));
    object_create(youwin, gs, youwin_ani->start_pos, vec2f_create(0,0));
    object_set_stl(youwin, bk_get_stl(&scene->bk_data));
    object_set_animation(youwin, youwin_ani);
    object_set_finish_cb(youwin, scene_youwin_anim_done);
    game_state_add_object(gs, youwin, RENDER_LAYER_TOP);

    // This will release HARs for action
    arena->state = ARENA_STATE_ENDING;
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
    arena_local *arena = scene_get_userdata(scene);
    animation *youlose_ani = &bk_get_info(&scene->bk_data, 8)->ani;
    object *youlose = malloc(sizeof(object));
    object_create(youlose, gs, youlose_ani->start_pos, vec2f_create(0,0));
    object_set_stl(youlose, bk_get_stl(&scene->bk_data));
    object_set_animation(youlose, youlose_ani);
    object_set_finish_cb(youlose, scene_youlose_anim_done);
    game_state_add_object(gs, youlose, RENDER_LAYER_TOP);

    // This will release HARs for action
    arena->state = ARENA_STATE_ENDING;
}

void arena_end(scene *sc) {
    // after the match ends, switch the newsroom
    DEBUG("switching to newsroom");
    //arena_local *local = scene_get_userdata(sc);
    game_state *gs = sc->gs;

    // XXX TODO take victory pose screenshot for the newsroom
    game_state_set_next(gs, SCENE_NEWSROOM);
}

int is_netplay(scene *scene) {
    if(game_state_get_player(scene->gs, 0)->ctrl->type == CTRL_TYPE_NETWORK ||
            game_state_get_player(scene->gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
        return 1;
    }
    return 0;
}

void arena_maybe_sync(scene *scene, int need_sync) {
    game_state *gs = scene->gs;
    game_player *player1 = game_state_get_player(gs, 0);
    game_player *player2 = game_state_get_player(gs, 1);

    if (need_sync && gs->role == ROLE_SERVER && (player1->ctrl->type == CTRL_TYPE_NETWORK ||  player2->ctrl->type == CTRL_TYPE_NETWORK)) {
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

void arena_hit_hook(int hittee, int hitter, af_move *move, void *data) {
    scene *scene = data;
    chr_score *score;
    chr_score *otherscore;
    object *hit_har;
    har *h;
    if (hitter == 0) {
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

void arena_recover_hook(int player_id, void *data) {
    scene *scene = data;
    chr_score *score;
    object *o_har;
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

    if (is_netplay(scene) && scene->gs->role == ROLE_CLIENT) {
        // only the server keeps score
        return;
    }
    har_install_hit_hook(har1, &arena_hit_hook, scene);
    har_install_hit_hook(har2, &arena_hit_hook, scene);

    har_install_recover_hook(har1, &arena_recover_hook, scene);
    har_install_recover_hook(har2, &arena_recover_hook, scene);
}


// -------- Scene callbacks --------

void arena_free(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(scene->gs, i);
        game_player_set_har(player, NULL);
        game_player_set_ctrl(player, NULL);
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
    if (i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
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
                /*object_set_spawn_cb(obj, cb_scene_spawn_object, (void*)scene);*/
                /*object_set_destroy_cb(obj, cb_scene_destroy_object, (void*)scene);*/
                hazard_create(obj, scene);
                game_state_add_object(scene->gs, obj, RENDER_LAYER_BOTTOM);
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


                DEBUG("Arena tick: Hazard with probability %d started.", info->probability, info->ani.id);
                changed++;
            }
        }
    }

    arena_maybe_sync(scene, changed);
}

void arena_tick(scene *scene) {
    arena_local *local = scene_get_userdata(scene);
    game_state *gs = scene->gs;
    game_player *player1 = game_state_get_player(gs, 0);
    game_player *player2 = game_state_get_player(gs, 1);

    // Handle scrolling score texts
    chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 0)));
    chr_score_tick(game_player_get_score(game_state_get_player(scene->gs, 1)));

    // Handle menu, if visible
    if(!local->menu_visible) {
        // Turn the HARs to face the enemy
        object *obj_har1,*obj_har2;
        obj_har1 = game_player_get_har(game_state_get_player(scene->gs, 0));
        obj_har2 = game_player_get_har(game_state_get_player(scene->gs, 1));
        har *har1, *har2;
        har1 = obj_har1->userdata;
        har2 = obj_har2->userdata;

        // keep enemy positions in sync
        // TODO can we do this with pointers instead?
        obj_har1->animation_state.enemy_x = obj_har2->pos.x;
        obj_har1->animation_state.enemy_y = obj_har2->pos.y;
        obj_har2->animation_state.enemy_x = obj_har1->pos.x;
        obj_har2->animation_state.enemy_y = obj_har1->pos.y;

        har1->delay = ceil(player2->ctrl->rtt / 2.0f);
        har2->delay = ceil(player1->ctrl->rtt / 2.0f);

        if(local->state != ARENA_STATE_ENDING && local->state != ARENA_STATE_STARTING) {
            arena_spawn_hazard(scene);
        }

        if (
                (har1->state == STATE_STANDING || har_is_crouching(har1) || har_is_walking(har1) || har1->state == STATE_STUNNED) &&
                (har2->state == STATE_STANDING || har_is_crouching(har1) || har_is_walking(har2) || har2->state == STATE_STUNNED)) {
            // XXX if the other har is stunned, turn the non stunned HAR to face it, but never turn a stunned HAR
            vec2i pos1, pos2;
            pos1 = object_get_pos(obj_har1);
            pos2 = object_get_pos(obj_har2);
            if(pos1.x > pos2.x) {
                if(object_get_direction(obj_har1) == OBJECT_FACE_RIGHT || object_get_direction(obj_har2) == OBJECT_FACE_LEFT) {
                    if (har1->state != STATE_STUNNED) {
                        object_set_direction(obj_har1, OBJECT_FACE_LEFT);
                    }
                    if (har2->state != STATE_STUNNED) {
                        object_set_direction(obj_har2, OBJECT_FACE_RIGHT);
                    }
                }
            } else if(pos1.x < pos2.x) {
                if(object_get_direction(obj_har1) == OBJECT_FACE_LEFT || object_get_direction(obj_har2) == OBJECT_FACE_RIGHT) {
                    if (har1->state != STATE_STUNNED) {
                        object_set_direction(obj_har1, OBJECT_FACE_RIGHT);
                    }
                    if (har2->state != STATE_STUNNED) {
                        object_set_direction(obj_har2, OBJECT_FACE_LEFT);
                    }
                }
            }
        }

        // Display you win/lose animation
        if(local->state != ARENA_STATE_ENDING) {
            // Har victory animation
            if(har2->health <= 0 && har2->endurance <= 0 && har2->state == STATE_DEFEAT) {
                // XXX need a smarter way to detect if a player is networked or local
                if(game_state_get_player(gs, 0)->ctrl->type != CTRL_TYPE_NETWORK &&
                   game_state_get_player(gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
                    scene_youwin_anim_start(scene->gs);
                } else if(game_state_get_player(gs, 0)->ctrl->type == CTRL_TYPE_NETWORK &&
                          game_state_get_player(gs, 1)->ctrl->type != CTRL_TYPE_NETWORK) {
                    scene_youlose_anim_start(scene->gs);
                } else {
                    scene_youwin_anim_start(scene->gs);
                }
                har_set_ani(obj_har1, ANIM_VICTORY, 1);
                har_set_ani(obj_har2, ANIM_DEFEAT, 1);
                har1->state = STATE_VICTORY;
                har1->executing_move = 1;
                har2->state = STATE_DEFEAT;
                object_set_vel(obj_har2, vec2f_create(0, 0));
                object_set_gravity(obj_har2, 0);
                chr_score *score = game_player_get_score(game_state_get_player(gs, 0));
                arena_maybe_sync(scene,
                        chr_score_interrupt(score, object_get_pos(obj_har1)));
                local->ending_ticks = 0;
            } else if(har1->health <= 0 && har1->endurance <= 0 && har1->state == STATE_DEFEAT) {
                if(game_state_get_player(gs, 0)->ctrl->type != CTRL_TYPE_NETWORK &&
                   game_state_get_player(gs, 1)->ctrl->type == CTRL_TYPE_NETWORK) {
                    scene_youlose_anim_start(scene->gs);
                } else if(game_state_get_player(gs, 0)->ctrl->type == CTRL_TYPE_NETWORK &&
                          game_state_get_player(gs, 1)->ctrl->type != CTRL_TYPE_NETWORK) {
                    scene_youwin_anim_start(scene->gs);
                } else {
                    if(game_player_get_selectable(game_state_get_player(gs, 1))) {
                        // XXX in two player mode, "you win" should always be displayed
                        scene_youwin_anim_start(scene->gs);
                    } else {
                        scene_youlose_anim_start(scene->gs);
                    }
                }
                har_set_ani(obj_har2, ANIM_VICTORY, 1);
                har_set_ani(obj_har1, ANIM_DEFEAT, 1);
                har2->state = STATE_VICTORY;
                har2->executing_move = 1;
                har1->state = STATE_DEFEAT;
                object_set_vel(obj_har1, vec2f_create(0, 0));
                object_set_gravity(obj_har1, 0);
                chr_score *score = game_player_get_score(game_state_get_player(gs, 1));
                arena_maybe_sync(scene,
                        chr_score_interrupt(score, object_get_pos(obj_har2)));
                local->ending_ticks = 0;
            }
        } else if(local->state == ARENA_STATE_ENDING) {
            // increment tick if the HAR isn't doing scrap/destruction and if the score isn't scrolling
            if(har1->state == STATE_SCRAP || har1->state == STATE_DESTRUCTION ||
               har2->state == STATE_SCRAP || har2->state == STATE_DESTRUCTION) {
                // spare some ticks to show the victory pose after doing a scrap/desstruction
                local->ending_ticks = 80;

            } else if(chr_score_get_num_texts(game_player_get_score(game_state_get_player(gs, 0))) == 0 &&
                      chr_score_get_num_texts(game_player_get_score(game_state_get_player(gs, 1))) == 0) {
                // only tick if the score isn't scrolling
                local->ending_ticks++;
            }
            if(local->ending_ticks == 150) {
                arena_end(scene);
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
                    object_tick(scrap);
                    scrap->cast_shadow = 1;
                    scrap_create(scrap);
                    game_state_add_object(gs, scrap, RENDER_LAYER_TOP);
                }
            }
        }
    }

    int need_sync = 0;
    // allow enemy HARs to move during a network game
    need_sync += arena_handle_events(scene, player1, player1->ctrl->extra_events);
    need_sync += arena_handle_events(scene, player2, player2->ctrl->extra_events);
    arena_maybe_sync(scene, need_sync);
}

void arena_input_tick(scene *scene) {
    arena_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    game_player *player2 = game_state_get_player(scene->gs, 1);

    if(!local->menu_visible) {
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
}

int arena_event(scene *scene, SDL_Event *e) {
    arena_local *local = scene_get_userdata(scene);

    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            if (!local->menu_visible) {
                local->menu_visible = 1;
            } else {
                local->menu_visible = 0;
            }
            return 1;
        }
        break;
    }
    if(local->menu_visible) {
        return menu_handle_event(&local->game_menu, e);
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
    font_render(&font_small, buf, 120, 8, TEXT_COLOR);
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
        font_render(&font_small, lang_get(player[0]->pilot_id+20), 5, 19, TEXT_COLOR);
        font_render(&font_small, lang_get((player[0]->har_id - HAR_JAGUAR)+31), 5, 26, TEXT_COLOR);

        int p2len = (strlen(lang_get(player[1]->pilot_id+20))-1) * font_small.w;
        int h2len = (strlen(lang_get((player[1]->har_id - HAR_JAGUAR)+31))-1) * font_small.w;
        font_render(&font_small, lang_get(player[1]->pilot_id+20), 315-p2len, 19, TEXT_COLOR);
        font_render(&font_small, lang_get((player[1]->har_id - HAR_JAGUAR)+31), 315-h2len, 26, TEXT_COLOR);

        // Render score stuff
        chr_score_render(game_player_get_score(player[0]));
        chr_score_render(game_player_get_score(player[1]));

        // render ping, if player is networked
        if (player[0]->ctrl->type == CTRL_TYPE_NETWORK) {
            sprintf(buf, "ping %u", player[0]->ctrl->rtt);
            font_render(&font_small, buf, 5, 40, TEXT_COLOR);
        }
        if (player[1]->ctrl->type == CTRL_TYPE_NETWORK) {
            sprintf(buf, "ping %u", player[1]->ctrl->rtt);
            font_render(&font_small, buf, 315-(strlen(buf)*font_small.w), 40, TEXT_COLOR);
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
    
    // Handle music playback
    music_stop();
    char music_filename[64];
    switch(scene->bk_data.file_id) {
        case 8:   get_filename_by_id(PSM_ARENA0, music_filename); break;
        case 16:  get_filename_by_id(PSM_ARENA1, music_filename); break;
        case 32:  get_filename_by_id(PSM_ARENA2, music_filename); break;
        case 64:  get_filename_by_id(PSM_ARENA3, music_filename); break;
        case 128: get_filename_by_id(PSM_ARENA4, music_filename); break;
    }
    music_play(music_filename);
    music_set_volume(settings_get()->sound.music_vol/10.0f);

    // Initialize local struct
    local = malloc(sizeof(arena_local));
    scene_set_userdata(scene, local);

    // Set correct state
    local->state = ARENA_STATE_STARTING;
    local->ending_ticks = 0;
    local->rein_enabled = 0;

    // Initial har data
    vec2i pos[2];
    int dir[2] = {OBJECT_FACE_RIGHT, OBJECT_FACE_LEFT};
    pos[0] = vec2i_create(60, 190);
    pos[1] = vec2i_create(260, 190);

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
    chr_score_reset(game_player_get_score(_player[0]));
    chr_score_reset(game_player_get_score(_player[1]));

    // TODO: Do something about this hack!
    scene->bk_data.sound_translation_table[14] = 10; // READY

    // Start READY animation
    animation *ready_ani = &bk_get_info(&scene->bk_data, 11)->ani;
    object *ready = malloc(sizeof(object));
    object_create(ready, scene->gs, ready_ani->start_pos, vec2f_create(0,0));
    object_set_stl(ready, scene->bk_data.sound_translation_table);
    object_set_animation(ready, ready_ani);
    object_set_finish_cb(ready, scene_ready_anim_done);
    game_state_add_object(scene->gs, ready, RENDER_LAYER_TOP);

    // Callbacks
    scene_set_event_cb(scene, arena_event);
    scene_set_free_cb(scene, arena_free);
    scene_set_tick_cb(scene, arena_tick);
    scene_set_input_poll_cb(scene, arena_input_tick);
    scene_set_render_overlay_cb(scene, arena_render_overlay);

    // All done!
    return 0;
}
