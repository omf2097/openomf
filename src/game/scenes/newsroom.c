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
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/random.h"
#include "utils/str.h"
#include "video/surface.h"
#include "video/video.h"

// newsroom text starts at 87 in english.dat
// there are 24*2 texts in total
#define NEWSROOM_TEXT 87

typedef struct newsroom_local_t {
    int news_id;
    int screen;
    surface news_bg;
    str news_str;
    str pilot1, pilot2;
    str har1, har2;
    int sex1, sex2;
    int won;
    bool champion;
    dialog continue_dialog;
} newsroom_local;

const char *object_pronoun(int sex) {
    if(sex == PILOT_SEX_MALE) {
        return "Him";
    }
    return "Her";
}

const char *subject_pronoun(int sex) {
    if(sex == PILOT_SEX_MALE) {
        return "He";
    }
    return "She";
}

void newsroom_fixup_str(newsroom_local *local) {
    /*
     * Substitution table

       1= Player1 - Crystal
       2= Player2 - Steffan
       3= HAR1 - jaguar
       4= HAR2 - shadow
       5= Arena - power plant
       6= His/Her P1 - Her
       7= Him/Her P1 - Her
       8= He/She P1 - She
       10= Him/Her P2 - Him
       11= He/She P2 - He
    */

    unsigned int translation_id;

    if(local->champion && local->screen >= 2) {
        translation_id = 79;
    } else {
        translation_id = NEWSROOM_TEXT + local->news_id + min2(local->screen, 1);
    }

    str tmp;
    str_from_c(&tmp, lang_get(translation_id));
    str_replace(&tmp, "~11", subject_pronoun(local->sex2), -1);
    str_replace(&tmp, "~10", object_pronoun(local->sex2), -1);
    str_replace(&tmp, "~9", "WTF", -1);
    str_replace(&tmp, "~8", subject_pronoun(local->sex1), -1);
    str_replace(&tmp, "~7", object_pronoun(local->sex1), -1);
    str_replace(&tmp, "~6", "The", -1);
    str_replace(&tmp, "~5", "Stadium", -1);
    str_replace(&tmp, "~4", str_c(&local->har2), -1);
    str_replace(&tmp, "~3", str_c(&local->har1), -1);
    str_replace(&tmp, "~2", str_c(&local->pilot2), -1);
    str_replace(&tmp, "~1", str_c(&local->pilot1), -1);

    str_free(&local->news_str);
    str_from(&local->news_str, &tmp);
    str_free(&tmp);
}

void newsroom_set_names(newsroom_local *local, const char *pilot1, const char *pilot2, const char *har1,
                        const char *har2, int sex1, int sex2) {

    str_from_c(&local->pilot1, pilot1);
    str_from_c(&local->pilot2, pilot2);
    str_from_c(&local->har1, har1);
    str_from_c(&local->har2, har2);
    local->sex1 = sex1;
    local->sex2 = sex2;

    // Remove the whitespace at the end of pilots name
    str_rstrip(&local->pilot1);
    str_rstrip(&local->pilot2);
}

// newsroom callbacks
void newsroom_free(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);
    surface_free(&local->news_bg);
    str_free(&local->news_str);
    str_free(&local->pilot1);
    str_free(&local->pilot2);
    str_free(&local->har1);
    str_free(&local->har2);
    dialog_free(&local->continue_dialog);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void newsroom_static_tick(scene *scene, int paused) {
    newsroom_local *local = scene_get_userdata(scene);
    dialog_tick(&local->continue_dialog);
}

void newsroom_overlay_render(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    // Render screen capture
    har_screencaps *caps = &(game_state_get_player(scene->gs, (local->won ? 0 : 1))->screencaps);
    if(local->screen == 0) {
        if(caps->ok[SCREENCAP_POSE])
            video_draw_size(&caps->cap[SCREENCAP_POSE], 165, 15, SCREENCAP_W, SCREENCAP_H);
    } else {
        if(caps->ok[SCREENCAP_BLOW])
            video_draw_size(&caps->cap[SCREENCAP_BLOW], 165, 15, SCREENCAP_W, SCREENCAP_H);
    }

    // Render text
    if(str_size(&local->news_str) > 0) {
        video_draw(&local->news_bg, 20, 140);
        text_settings tconf_yellow;
        text_defaults(&tconf_yellow);
        tconf_yellow.font = FONT_SMALL;
        tconf_yellow.cforeground = COLOR_YELLOW;
        tconf_yellow.shadow = TEXT_SHADOW_RIGHT | TEXT_SHADOW_BOTTOM;
        tconf_yellow.cshadow = 202;
        tconf_yellow.halign = TEXT_CENTER;
        text_render(&tconf_yellow, TEXT_DEFAULT, 30, 150, 250, 6, str_c(&local->news_str));
    }

    // Dialog
    if(dialog_is_visible(&local->continue_dialog)) {
        dialog_render(&local->continue_dialog);
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

void newsroom_input_tick(scene *scene) {
    newsroom_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i) {
        do {
            if(i->type == EVENT_TYPE_ACTION) {
                if(dialog_is_visible(&local->continue_dialog)) {
                    dialog_event(&local->continue_dialog, i->event_data.action);
                } else if(i->event_data.action == ACT_ESC || i->event_data.action == ACT_KICK ||
                          i->event_data.action == ACT_PUNCH) {
                    local->screen++;
                    newsroom_fixup_str(local);

                    if((local->screen >= 2 && !local->champion) || local->screen >= 3) {
                        if(local->won || player1->chr) {
                            // pick a new player
                            game_player *p1 = game_state_get_player(scene->gs, 0);
                            game_player *p2 = game_state_get_player(scene->gs, 1);
                            if(p1->chr) {
                                // clear the opponent as a signal to display plug on the VS
                                p2->pilot = NULL;
                                // also zero out the p2 wins so the game doesn't think
                                // we keep losing
                                p2->sp_wins = 0;
                            } else {
                                DEBUG("wins are %d", p1->sp_wins);
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
                                    sd_pilot_set_player_color(p2->pilot, TERTIARY, p.colors[0]);
                                    sd_pilot_set_player_color(p2->pilot, SECONDARY, p.colors[1]);
                                    sd_pilot_set_player_color(p2->pilot, PRIMARY, p.colors[2]);

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
    controller_free_chain(p1);
}

int pilot_sex(int pilot_id) {
    switch(pilot_id) {
        case 0:
            // Crystal
        case 7:
            // Angel
        case 8:
            // Cossette
            return PILOT_SEX_FEMALE;
        default:
            // everyone else is male
            return PILOT_SEX_MALE;
    }
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
    menu_background_create(&local->news_bg, 280, 50);

    game_player *p1 = game_state_get_player(scene->gs, 0);
    game_player *p2 = game_state_get_player(scene->gs, 1);

    int health = 0;
    if(p2->sp_wins > 0) {
        // AI won, player lost
        local->won = 0;
        health = game_player_get_score(p2)->health;
    } else {
        local->won = 1;
        if(p1->chr && p1->chr->pilot.rank == 1) {
            local->champion = true;
        }
        health = game_player_get_score(p1)->health;
    }

    DEBUG("health is %d", health);

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

    // XXX TODO get the real sex of pilot
    // XXX TODO strip spaces from the end of the pilots name
    // XXX TODO set winner/loser names properly
    if(p1->chr) {
        // TODO pilot genders
        newsroom_set_names(local, p1->pilot->name, p2->pilot->name, har_get_name(p1->pilot->har_id),
                           har_get_name(p2->pilot->har_id), pilot_sex(p1->pilot->pilot_id),
                           pilot_sex(p2->pilot->pilot_id));
    } else {
        newsroom_set_names(local, lang_get(20 + p1->pilot->pilot_id), lang_get(20 + p2->pilot->pilot_id),
                           har_get_name(p1->pilot->har_id), har_get_name(p2->pilot->har_id),
                           pilot_sex(p1->pilot->pilot_id), pilot_sex(p2->pilot->pilot_id));
    }
    newsroom_fixup_str(local);

    // Continue Dialog
    dialog_create(&local->continue_dialog, DIALOG_STYLE_YES_NO, "DO YOU WISH TO CONTINUE?", 72, 60);
    local->continue_dialog.userdata = scene;
    local->continue_dialog.clicked = newsroom_continue_dialog_clicked;

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
