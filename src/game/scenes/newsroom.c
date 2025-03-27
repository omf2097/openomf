#include "game/scenes/newsroom.h"
#include "audio/audio.h"
#include "game/gui/dialog.h"
#include "game/gui/menu_background.h"
#include "game/gui/text_render.h"
#include "game/utils/settings.h"
#include "resources/ids.h"
#include "resources/languages.h"
#include "resources/pilots.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "utils/str.h"
#include "video/surface.h"
#include "video/video.h"

typedef struct newsroom_local_t {
    int news_id;
    int screen;
    surface news_bg1;
    surface news_bg2;
    str news_str;
    str pilot1, pilot2;
    int har1, har2;
    int sex1, sex2;
    int won;
    bool champion;
    sd_pilot *challenger;
    dialog continue_dialog;
    dialog accept_challenge_dialog;
} newsroom_local;

// their
const char *possessive_pronoun(int sex) {
    return lang_get(LANG_STR_PRONOUN + sex);
}

// them
const char *object_pronoun(int sex) {
    return lang_get(LANG_STR_PRONOUN + 2 + sex);
}

// they
const char *subject_pronoun(int sex) {
    return lang_get(LANG_STR_PRONOUN + 4 + sex);
}

char const *pronoun_strip(char const *pronoun, char *buf, size_t buf_size) {
    size_t pronoun_len = strlen(pronoun);
    while(pronoun_len && pronoun[pronoun_len - 1] == '\n') {
        pronoun_len--;
    }
    if(pronoun_len >= buf_size)
        pronoun_len = buf_size - 1;
    memcpy(buf, pronoun, pronoun_len);
    buf[pronoun_len] = '\0';
    return buf;
}

static void newsroom_fixup_capitalization(str *tmp) {
    // XXX non-const str_c variant?
    char *it = (char *)str_c(tmp);
    while(it && *it) {
        unsigned char c = *it;
        // XXX undone: capitalization outside of ASCII range
        if(c < 0x7F)
            // capitalize the letter
            *it = toupper(c);

        // find next char that has two spaces in front
        it = strstr(it + 1, "  ");
        if(it == NULL)
            return;
        it += 2;
    }
}

void newsroom_fixup_str(newsroom_local *local) {
    /*
     * Substitution table

       1= Player1 - Crystal
       2= Player2 - Steffan
       3= HAR1 - Jaguar
       4= HAR2 - Shadow
       5= Arena - Stadium
       6= P1 Possessive pronoun - Her
       7= P1 Objective pronoun  - Her
       8= P1 Subjective pronoun - She
       9= P2 Possessive pronoun - His
       10= P2 Objective pronoun  - Him
       11= P2 Subjective pronoun - He
    */

    unsigned int translation_id;

    if(local->champion && local->screen >= 2) {
        translation_id = LANG_STR_NEWSROOM_NEWCHAMPION;
    } else if(local->challenger && local->screen == 0) {
        translation_id = LANG_STR_NEWSROOM_CHALLENGER1;
    } else if(local->challenger && local->screen >= 1) {
        translation_id = LANG_STR_NEWSROOM_CHALLENGER2;
    } else {
        translation_id = LANG_STR_NEWSROOM_TEXT + local->news_id + min2(local->screen, 1);
    }

    char scratch[9];
    str tmp;
    str_from_c(&tmp, lang_get(translation_id));
    str_replace(&tmp, "~11", pronoun_strip(subject_pronoun(local->sex2), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~10", pronoun_strip(object_pronoun(local->sex2), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~9", pronoun_strip(possessive_pronoun(local->sex2), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~8", pronoun_strip(subject_pronoun(local->sex1), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~7", pronoun_strip(object_pronoun(local->sex1), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~6", pronoun_strip(possessive_pronoun(local->sex1), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~5", "Stadium", -1);
    str_replace(&tmp, "~4", pronoun_strip(lang_get(local->har2 + LANG_STR_HAR), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~3", pronoun_strip(lang_get(local->har1 + LANG_STR_HAR), scratch, sizeof scratch), -1);
    str_replace(&tmp, "~2", str_c(&local->pilot2), -1);
    str_replace(&tmp, "~1", str_c(&local->pilot1), -1);

    newsroom_fixup_capitalization(&tmp);

    str_free(&local->news_str);
    local->news_str = tmp;
}

void newsroom_set_names(newsroom_local *local, const char *pilot1, const char *pilot2, int har1, int har2, int sex1,
                        int sex2) {

    str_from_c(&local->pilot1, pilot1);
    str_from_c(&local->pilot2, pilot2);
    local->har1 = har1;
    local->har2 = har2;
    local->sex1 = sex1;
    local->sex2 = sex2;

    // Remove the whitespace at the end of pilots name
    str_rstrip(&local->pilot1);
    str_rstrip(&local->pilot2);
}

// newsroom callbacks
void newsroom_free(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);
    surface_free(&local->news_bg1);
    surface_free(&local->news_bg2);
    str_free(&local->news_str);
    str_free(&local->pilot1);
    str_free(&local->pilot2);
    dialog_free(&local->continue_dialog);
    dialog_free(&local->accept_challenge_dialog);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void newsroom_static_tick(scene *scene, int paused) {
    newsroom_local *local = scene_get_userdata(scene);
    dialog_tick(&local->continue_dialog);
    dialog_tick(&local->accept_challenge_dialog);
}

void newsroom_overlay_render(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    if(!local->challenger) {
        // Render screen capture
        har_screencaps *caps = &(game_state_get_player(scene->gs, (local->won ? 0 : 1))->screencaps);
        if(local->screen == 0) {
            if(caps->ok[SCREENCAP_POSE])
                video_draw_size(&caps->cap[SCREENCAP_POSE], 165, 15, SCREENCAP_W, SCREENCAP_H);
        } else {
            if(caps->ok[SCREENCAP_BLOW])
                video_draw_size(&caps->cap[SCREENCAP_BLOW], 165, 15, SCREENCAP_W, SCREENCAP_H);
        }
    }

    // Render text
    if(str_size(&local->news_str) > 0) {
        video_draw_remap(&local->news_bg1, 20, 131, 4, 1, 0);
        video_draw(&local->news_bg2, 20, 131);
        text_settings tconf_yellow;
        text_defaults(&tconf_yellow);
        tconf_yellow.font = FONT_BIG;
        tconf_yellow.cforeground = COLOR_YELLOW;
        tconf_yellow.shadow = TEXT_SHADOW_NONE;
        tconf_yellow.cshadow = 202;
        tconf_yellow.halign = TEXT_CENTER;
        tconf_yellow.valign = TEXT_MIDDLE;
        tconf_yellow.lspacing = 1;
        tconf_yellow.strip_leading_whitespace = false;
        tconf_yellow.strip_trailing_whitespace = true;
        tconf_yellow.max_lines = 9;
        text_render_mode(&tconf_yellow, TEXT_DEFAULT, 34, 155, 250, 6, str_c(&local->news_str));
    }

    // If the player has just become a new champion, show the sprite on top of the photo.
    if(local->champion && local->screen >= 2) {
        animation *photo_overlays = &bk_get_info(scene->bk_data, 4)->ani;
        sprite *new_champion = animation_get_sprite(photo_overlays, 1);
        video_draw(new_champion->data, new_champion->pos.x, new_champion->pos.y);
    }

    if(local->challenger) {
        animation *photo_overlays = &bk_get_info(scene->bk_data, 4)->ani;
        sprite *new_champion = animation_get_sprite(photo_overlays, 0);
        video_draw(new_champion->data, new_champion->pos.x, new_champion->pos.y);
    }

    // Dialog
    if(dialog_is_visible(&local->continue_dialog)) {
        dialog_render(&local->continue_dialog);
    }

    if(dialog_is_visible(&local->accept_challenge_dialog)) {
        dialog_render(&local->accept_challenge_dialog);
    }
}

void newsroom_continue_dialog_clicked(dialog *dlg, dialog_result result) {
    scene *sc = dlg->userdata;
    if(result == DIALOG_RESULT_NO) {
        game_state_set_next(sc->gs, SCENE_SCOREBOARD);
    } else if(result == DIALOG_RESULT_YES_OK) {
        // Resetting p2->sp_wins here allows the game to progress,
        // otherwise you get stuck with the same opponent
        game_player *p1 = game_state_get_player(sc->gs, 0);
        game_player *p2 = game_state_get_player(sc->gs, 1);
        p2->sp_wins = 0;
        // score is reset by the scoreboard scene
        chr_score_reset_wins(game_player_get_score(p1));
        game_state_set_next(sc->gs, SCENE_SCOREBOARD);
        sc->gs->next_next_id = SCENE_VS;
    }
}

void newsroom_accept_challenge_dialog_clicked(dialog *dlg, dialog_result result) {
    scene *sc = dlg->userdata;
    newsroom_local *local = scene_get_userdata(sc);
    game_player *p2 = game_state_get_player(sc->gs, 1);
    if(result == DIALOG_RESULT_NO) {
        p2->pilot = NULL;
        // go back to the mechlab in disgrace
        game_state_set_next(sc->gs, SCENE_MECHLAB);
    } else if(result == DIALOG_RESULT_YES_OK) {
        p2->pilot = local->challenger;
        game_state_set_next(sc->gs, SCENE_VS);
    }
}

void newsroom_input_tick(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    game_player *p1 = game_state_get_player(scene->gs, 0);
    game_player *p2 = game_state_get_player(scene->gs, 1);

    ctrl_event *event = NULL, *i;
    game_state_menu_poll(scene->gs, &event);
    i = event;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(dialog_is_visible(&local->continue_dialog)) {
                    dialog_event(&local->continue_dialog, i->event_data.action);
                } else if(dialog_is_visible(&local->accept_challenge_dialog)) {
                    dialog_event(&local->accept_challenge_dialog, i->event_data.action);
                } else if(i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                          i->event_data.action == ACT_PUNCH) {
                    local->screen++;
                    newsroom_fixup_str(local);

                    if(local->challenger) {
                        if(local->screen >= 2) {
                            // show dialog asking if they want to accept the challenge
                            dialog_show(&local->accept_challenge_dialog, 1);
                        }
                    } else if((local->screen >= 2 && !local->champion) || local->screen >= 3) {
                        if(local->won || p1->chr) {
                            // pick a new player
                            if(p1->chr) {
                                // clear the opponent as a signal to display plug on the VS
                                p2->pilot = NULL;
                                // also zero out the p2 wins so the game doesn't think
                                // we keep losing
                                p2->sp_wins = 0;
                            } else {
                                log_debug("wins are %d", p1->sp_wins);
                                if(p1->sp_wins == (4094 ^ (2 << p1->pilot->pilot_id))) {
                                    // won the game
                                    game_state_set_next(scene->gs, SCENE_END);
                                } else {
                                    if(p1->sp_wins == (2046 ^ (2 << p1->pilot->pilot_id))) {
                                        // everyone but kreissack
                                        p2->pilot->pilot_id = PILOT_KREISSACK;
                                        p2->pilot->har_id = HAR_NOVA;
                                    } else {
                                        // pick an opponent we have not yet beaten
                                        while(1) {
                                            int i = rand_int(10);
                                            if((2 << i) & p1->sp_wins || i == p1->pilot->pilot_id) {
                                                continue;
                                            }
                                            p2->pilot->pilot_id = i;
                                            p2->pilot->har_id = rand_int(10);
                                            break;
                                        }
                                    }
                                    pilot p;
                                    pilot_get_info(&p, p2->pilot->pilot_id);
                                    p2->pilot->endurance = p.endurance;
                                    p2->pilot->power = p.power;
                                    p2->pilot->agility = p.agility;
                                    p2->pilot->sex = p.sex;
                                    sd_pilot_set_player_color(p2->pilot, TERTIARY, p.colors[0]);
                                    sd_pilot_set_player_color(p2->pilot, SECONDARY, p.colors[1]);
                                    sd_pilot_set_player_color(p2->pilot, PRIMARY, p.colors[2]);

                                    strncpy_or_abort(p2->pilot->name, lang_get(p2->pilot->pilot_id + 20),
                                                     sizeof(p2->pilot->name));
                                    // TODO: lang: remove (the need for) newline stripping
                                    // 1player name strings end in a newline...
                                    if(p2->pilot->name[strlen(p2->pilot->name) - 1] == '\n') {
                                        p2->pilot->name[strlen(p2->pilot->name) - 1] = 0;
                                    }

                                    // make a new AI controller
                                    controller *ctrl = omf_calloc(1, sizeof(controller));
                                    controller_init(ctrl, scene->gs);
                                    sd_pilot *pilot = game_player_get_pilot(p2);
                                    ai_controller_create(ctrl, settings_get()->gameplay.difficulty, pilot,
                                                         p2->pilot->pilot_id);
                                    game_player_set_ctrl(p2, ctrl);
                                }
                            }
                            if(p1->chr && local->champion) {
                                game_state_set_next(scene->gs, p1->chr->cutscene);
                            } else {
                                game_state_set_next(scene->gs, SCENE_VS);
                            }
                        } else {
                            dialog_show(&local->continue_dialog, 1);
                        }
                    }
                }
            }
        } while((i = i->next));
    }
    controller_free_chain(event);
}

void newsroom_startup(scene *scene, int id, int *m_load, int *m_repeat) {
    switch(id) {
        case 5:
        case 6:
            *m_load = 1;
            return;
    }
}

int newsroom_create(scene *scene) {
    newsroom_local *local = omf_calloc(1, sizeof(newsroom_local));

    local->news_id = rand_int(24) * 2;
    local->screen = 0;
    local->champion = false;
    local->challenger = NULL;
    menu_transparent_bg_create(&local->news_bg1, 280, 55);
    menu_background_create(&local->news_bg2, 280, 55, MenuBackgroundNewsroom);

    game_player *p1 = game_state_get_player(scene->gs, 0);
    game_player *p2 = game_state_get_player(scene->gs, 1);

    if(p1->chr && p2->pilot && p2->pilot->only_fight_once) {
        // we beat an only fight once pilot, we never want to see them again
        // TODO figure out how the original disables them from fighting again
        // XXX just give them a very high rank for now?
        p2->pilot->rank = p1->chr->pilot.enemies_inc_unranked + 1;
    }

    int health = 0;
    if(p2->sp_wins > 0) {
        // AI won, player lost
        local->won = 0;
        health = game_player_get_score(p2)->health;
    } else {
        local->won = 1;
        fight_stats *fight_stats = &scene->gs->fight_stats;
        if(p1->chr && fight_stats->challenger) {
            // we came here from plug
            local->challenger = fight_stats->challenger;
            p2->pilot = local->challenger;
            fight_stats->challenger = NULL;
        } else if(p1->chr) {
            int health = game_player_get_score(p1)->health;
            // see if we have meet any unranked challenger criteria
            for(int k = p1->chr->pilot.enemies_ex_unranked - 1; k < p1->chr->pilot.enemies_inc_unranked; k++) {
                sd_pilot *p = &p1->chr->enemies[k]->pilot;
                // TODO this is wrong, unranked challengers have a rank in the OG, just a rank higher than
                // pilot.enemies_ex_unranked
                if(p->rank == 0) {
                    // check all the conditions, if they're non zero
                    if((!p->req_rank || p->req_rank == p1->pilot->rank) &&
                       (!p->req_max_rank || p->req_max_rank >= p1->pilot->rank) &&
                       (!p->req_vitality || p->req_vitality <= health) &&
                       (!p->req_accuracy || p->req_accuracy <= fight_stats->hit_miss_ratio[0]) &&
                       (!p->req_avg_dmg || p->req_avg_dmg <= fight_stats->average_damage[0]) &&
                       // in tournament mode, this should be a pointer we can compare
                       (!p->req_enemy || &p1->chr->enemies[p->req_enemy - 1]->pilot == p2->pilot) &&
                       (!p->req_difficulty || p->req_difficulty <= p1->pilot->difficulty) &&
                       // XXX apparently you cannot require a jaguar, see tournament compiler readme
                       (!p->req_fighter || p->req_fighter == p1->pilot->har_id) &&
                       (!p->req_scrap || (p->req_scrap && fight_stats->finish >= FINISH_SCRAP)) &&
                       (!p->req_destroy || (p->req_destroy && fight_stats->finish == FINISH_DESTRUCTION))) {
                        fight_stats->challenger = p;
                        log_debug("found challenger!");
                        // XXX we are going to assume the first match, from the lowest in the file, is the one to choose
                        break;
                    }
                }
            }
        }
        if(!local->challenger && p1->chr && p1->chr->pilot.rank == 1) {
            local->champion = true;
        }
        health = game_player_get_score(p1)->health;
    }

    log_debug("health is %d", health);

    // there's 3 random messages for
    // each situation for winner/loser
    // depending on the winner's health
    // > 75%, >50% >25% and >0%

    if(health > 75 && local->won == 1) {
        // player won with > 75%
        local->news_id = rand_int(3) * 2;
    } else if(health > 50 && local->won == 1) {
        // player won with > 50%
        local->news_id = 6 + rand_int(3) * 2;
    } else if(health > 25 && local->won == 1) {
        // player won with > 25%
        local->news_id = 12 + rand_int(3) * 2;
    } else if(local->won == 1) {
        // player won with > 0%
        local->news_id = 18 + rand_int(3) * 2;
    } else if(health > 75 && local->won == 0) {
        // opponent won with > 75%
        local->news_id = 24 + rand_int(3) * 2;
    } else if(health > 50 && local->won == 0) {
        // opponent won with > 50%
        local->news_id = 30 + rand_int(3) * 2;
    } else if(health > 25 && local->won == 0) {
        // opponent won with > 25%
        local->news_id = 36 + rand_int(3) * 2;
    } else {
        // opponent won with > 0%
        local->news_id = 42 + rand_int(3) * 2;
    }

    // p2 pilot must be set by now
    assert(p2->pilot != NULL);

    newsroom_set_names(local, p1->pilot->name, p2->pilot->name, p1->pilot->har_id, p2->pilot->har_id, p1->pilot->sex,
                       p2->pilot->sex);
    newsroom_fixup_str(local);

    // Continue Dialog
    dialog_create(&local->continue_dialog, DIALOG_STYLE_YES_NO, lang_get(213), 72, 60);
    dialog_create(&local->accept_challenge_dialog, DIALOG_STYLE_YES_NO, lang_get(76), 72, 60);
    local->continue_dialog.userdata = scene;
    local->continue_dialog.clicked = newsroom_continue_dialog_clicked;

    local->accept_challenge_dialog.userdata = scene;
    local->accept_challenge_dialog.clicked = newsroom_accept_challenge_dialog_clicked;

    for(int i = 0; i < 2; i++) {
        game_player *player = game_state_get_player(scene->gs, i);

        // load the player's colors into the palette
        palette_load_player_colors(&player->pilot->palette, i);
    }

    // Set callbacks
    scene_set_userdata(scene, local);
    scene_set_input_poll_cb(scene, newsroom_input_tick);
    scene_set_render_overlay_cb(scene, newsroom_overlay_render);
    scene_set_free_cb(scene, newsroom_free);
    scene_set_static_tick_cb(scene, newsroom_static_tick);
    scene_set_startup_cb(scene, newsroom_startup);

    // Start correct music
    audio_play_music(PSM_MENU);

    return 0;
}
