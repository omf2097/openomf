#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "formats/error.h"
#include "formats/tournament.h"
#include "game/game_state.h"
#include "game/gui/gui_frame.h"
#include "game/gui/label.h"
#include "game/gui/menu_background.h"
#include "game/gui/text/text.h"
#include "game/gui/textinput.h"
#include "game/gui/trn_menu.h"
#include "game/protos/object.h"
#include "game/protos/scene.h"
#include "game/scenes/mechlab.h"
#include "game/scenes/mechlab/lab_dash_main.h"
#include "game/scenes/mechlab/lab_dash_newplayer.h"
#include "game/scenes/mechlab/lab_dash_trnselect.h"
#include "game/scenes/mechlab/lab_menu_difficultyselect.h"
#include "game/scenes/mechlab/lab_menu_main.h"
#include "game/scenes/mechlab/lab_menu_select.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/pathmanager.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "video/video.h"

// Colors specific to palette used by mechlab
#define TEXT_PRIMARY_COLOR 0xFE
#define TEXT_SECONDARY_COLOR 0xFD
#define TEXT_DISABLED_COLOR 0xC0
#define TEXT_ACTIVE_COLOR 0xFF
#define TEXT_INACTIVE_COLOR 0xFE
#define TEXT_SHADOW_COLOR 0xC0

#define POPUP_TEXT_W 200
#define POPUP_TEXT_H 48
#define POPUP_BG_W (POPUP_TEXT_W + 40)
#define POPUP_BG_H (POPUP_TEXT_H)
#define POPUP_CENTERY 70

typedef struct {
    dashboard_type dashtype;
    object bg_obj[3];
    gui_frame *frame;
    gui_frame *dashboard;
    object *mech;
    dashboard_widgets dw;
    newplayer_widgets nw;
    trnselect_widgets tw;
    bool selling;
    component *hint;
    gui_theme theme; // Required by the hint component
    text *popup;
    surface popup_bg1;
    surface popup_bg2;
} mechlab_local;

bool mechlab_find_last_player(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    // Find last saved game ...
    game_player *p1 = game_state_get_player(scene->gs, 0);

    const char *last_name = settings_get()->tournament.last_name;
    if(last_name == NULL || strlen(last_name) == 0) {
        last_name = NULL;
    }

    // ... and attempt to load it, if one was found and we don't have one already loaded
    if(!p1->chr && last_name != NULL) {
        sd_chr_file *chr = omf_calloc(1, sizeof(sd_chr_file));
        int ret = sg_load(chr, last_name);
        if(ret != SD_SUCCESS) {
            omf_free(chr);
            log_error("Could not load saved game for playername '%s': %s!", last_name, sd_get_error(ret));
            last_name = NULL;
        } else {
            p1->chr = chr;
            log_debug("Loaded savegame for playername '%s'.", last_name);
        }
    }

    // Either initialize a new tournament if no savegame is found,
    // or just show old savegame stats directly if it was.
    local->dashtype = DASHBOARD_NONE;
    if(p1->chr == NULL) {
        log_debug("No previous savegame found");
        object_free(local->mech);
        omf_free(local->mech);
        p1->pilot->money = 0;
        p1->pilot->har_id = 0;
        return false;
    } else {
        log_debug("Previous savegame found; loading as default.");
        // Load HAR
        animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + p1->chr->pilot.har_id)->ani;
        object_free(local->mech);
        omf_free(local->mech);
        local->mech = omf_calloc(1, sizeof(object));
        object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(local->mech, initial_har_ani);
        object_set_repeat(local->mech, 1);
        object_dynamic_tick(local->mech);
        sd_pilot *old_pilot = game_player_get_pilot(p1);
        if(&p1->chr->pilot != old_pilot) {
            game_player_set_pilot(p1, &p1->chr->pilot);
        }
    }
    return true;
}

void mechlab_load_har(scene *scene, sd_pilot *pilot) {
    mechlab_local *local = scene_get_userdata(scene);
    animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + pilot->har_id)->ani;
    object_free(local->mech);
    omf_free(local->mech);
    local->mech = omf_calloc(1, sizeof(object));
    object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
    object_set_animation(local->mech, initial_har_ani);
    object_set_repeat(local->mech, 1);
    object_dynamic_tick(local->mech);
}

void mechlab_set_selling(scene *scene, bool selling) {
    mechlab_local *local = scene_get_userdata(scene);
    local->selling = selling;
}

bool mechlab_get_selling(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    return local->selling;
}

void mechlab_set_hint(scene *scene, const char *hint) {
    mechlab_local *local = scene_get_userdata(scene);
    label_set_text(local->hint, hint);
}

static void mechlab_mech_finished_cb(object *obj) {
    player_reset(obj);
    player_run(obj);
    object_set_halt(obj, 1);
}

void mechlab_spin_har(scene *scene, bool spin) {
    mechlab_local *local = scene_get_userdata(scene);
    if(spin) {
        object_set_halt_ticks(local->mech, local->mech->halt);
        object_set_repeat(local->mech, 1);
    } else {
        local->mech->halt_ticks = 0;
        object_set_finish_cb(local->mech, mechlab_mech_finished_cb);
        object_set_repeat(local->mech, 0);
    }
}

sd_chr_enemy *mechlab_next_opponent(scene *scene) {
    game_player *p1 = game_state_get_player(scene->gs, 0);
    if(p1->chr->pilot.rank == 1) {
        return NULL;
    }
    for(int i = 0; i < p1->chr->pilot.enemies_inc_unranked; i++) {
        if(p1->chr->enemies[i]->pilot.rank == p1->chr->pilot.rank - 1) {
            return p1->chr->enemies[i];
        }
    }
    return NULL;
}

void mechlab_free(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    game_player *player1 = game_state_get_player(scene->gs, 0);
    // save the character file
    if(player1->chr != NULL && sg_save(player1->chr) != SD_SUCCESS) {
        log_error("Failed to save pilot %s", player1->chr->pilot.name);
    }

    for(unsigned i = 0; i < N_ELEMENTS(local->bg_obj); i++) {
        object_free(&local->bg_obj[i]);
    }

    text_free(&local->popup);
    surface_free(&local->popup_bg1);
    surface_free(&local->popup_bg2);
    gui_frame_free(local->frame);
    gui_frame_free(local->dashboard);
    object_free(local->mech);
    component_free(local->hint);
    omf_free(local->mech);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void mechlab_enter_trnselect_menu(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_trnselect_select, &local->tw, lab_dash_trnselect_left,
                                             &local->tw, lab_dash_trnselect_right, &local->tw, lang_get(486), true);
    gui_frame_set_root(local->frame, menu);
    gui_frame_layout(local->frame);
}

component *mechlab_chrload_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_load, &local->dw, lab_dash_main_chr_left,
                                             &local->dw, lab_dash_main_chr_right, &local->dw, lang_get(225), true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_main_chr_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_main_chr_done);
    trnmenu_set_userdata(menu, &local->dw);
    return menu;
}

component *mechlab_chrdelete_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_delete, &local->dw, lab_dash_main_chr_left,
                                             &local->dw, lab_dash_main_chr_right, &local->dw, lang_get(226), true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_main_chr_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_main_chr_done);
    trnmenu_set_userdata(menu, &local->dw);
    return menu;
}

component *mechlab_sim_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_delete, &local->dw, lab_dash_sim_left, &local->dw,
                                             lab_dash_sim_right, &local->dw, lang_get(227), true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_sim_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_sim_done);
    trnmenu_set_userdata(menu, &local->dw);
    local->dashtype = DASHBOARD_SIM;
    return menu;
}

void mechlab_open_popup(scene *scene, char const *message) {
    mechlab_local *local = scene_get_userdata(scene);
    text_free(&local->popup);
    local->popup = text_create_from_c(message);
    text_set_font(local->popup, FONT_BIG);
    text_set_bounding_box(local->popup, POPUP_TEXT_W, POPUP_TEXT_H);
    text_set_vertical_align(local->popup, TEXT_ALIGN_MIDDLE);
    text_set_horizontal_align(local->popup, TEXT_ALIGN_CENTER);
}

void mechlab_update(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *p1 = game_state_get_player(scene->gs, 0);
    if(p1->pilot) {
        animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + p1->pilot->har_id)->ani;
        if(local->mech && object_get_animation(local->mech) != initial_har_ani) {
            object_free(local->mech);
            object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(local->mech, initial_har_ani);
            object_set_repeat(local->mech, 1);
            object_dynamic_tick(local->mech);
        }
        switch(local->dashtype) {
            // Dashboard with the gauges etc.
            case DASHBOARD_STATS:
                // Dashboard widgets struct is filled with pointer to the necessary components for easy access
                lab_dash_main_update(scene, &local->dw);
                break;
            case DASHBOARD_SIM:
                lab_dash_sim_update(scene, &local->dw, p1->pilot);
                break;
            default:
                break;
        }
    } else {
        object_free(local->mech);
        omf_free(local->mech);
    }
}

static void mechlab_theme(gui_theme *theme) {
    gui_theme_defaults(theme);
    theme->dialog.border_color = TEXT_MEDIUM_GREEN;
    theme->text.font = FONT_BIG;
    theme->text.primary_color = TEXT_PRIMARY_COLOR;
    theme->text.secondary_color = TEXT_SECONDARY_COLOR;
    theme->text.disabled_color = TEXT_DISABLED_COLOR;
    theme->text.active_color = TEXT_ACTIVE_COLOR;
    theme->text.inactive_color = TEXT_INACTIVE_COLOR;
    theme->text.shadow_color = TEXT_SHADOW_COLOR;
}

void mechlab_tick(scene *scene, int paused) {
    mechlab_local *local = scene_get_userdata(scene);

    if(local->popup) {
        return;
    }

    gui_frame_tick(local->frame);
    gui_frame_tick(local->dashboard);
    if(local->mech != NULL) {
        object_dynamic_tick(local->mech);
    }

    // Check if root is finished
    component *root = gui_frame_get_root(local->frame);
    if(trnmenu_is_finished(root)) {
        game_player *player1 = game_state_get_player(scene->gs, 0);
        if(local->dashtype == DASHBOARD_NEW_PLAYER) {
            char select_photo[64];
            snprintf(select_photo, sizeof(select_photo), lang_get(224), player1->pilot->name);
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_NEW_PIC);
            gui_frame_free(local->frame);
            gui_theme theme;
            mechlab_theme(&theme);
            local->frame = gui_frame_create(&theme, 0, 0, 320, 200);
            component *menu =
                lab_menu_select_create(scene, lab_dash_main_photo_select, &local->dw, lab_dash_main_photo_left,
                                       &local->dw, lab_dash_main_photo_right, &local->dw, select_photo, true);
            gui_frame_set_root(local->frame, menu);
            gui_frame_layout(local->frame);
        } else if(local->dashtype == DASHBOARD_SELECT_NEW_PIC) {
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_DIFFICULTY);
            gui_frame_free(local->frame);
            gui_theme theme;
            mechlab_theme(&theme);
            local->frame = gui_frame_create(&theme, 0, 0, 320, 200);
            component *menu = lab_menu_difficultyselect_create(scene);
            gui_frame_set_root(local->frame, menu);
            gui_frame_layout(local->frame);
        } else if(local->dashtype == DASHBOARD_SELECT_DIFFICULTY) {
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_TOURNAMENT);
            gui_frame_free(local->frame);
            gui_theme theme;
            mechlab_theme(&theme);
            local->frame = gui_frame_create(&theme, 0, 0, 320, 200);
            mechlab_enter_trnselect_menu(scene);
        } else if(local->dashtype == DASHBOARD_SELECT_TOURNAMENT) {
            sd_tournament_file *trn = lab_dash_trnselect_selected(&local->tw);
            if(player1->pilot->money < trn->registration_fee) {
                player1->pilot->money = 0;
            } else {
                player1->pilot->money = player1->pilot->money - trn->registration_fee;
            }
            sd_chr_file *oldchr = player1->chr;
            player1->chr = omf_calloc(1, sizeof(sd_chr_file));
            sd_chr_create(player1->chr);
            memcpy(&player1->chr->pilot, player1->pilot, sizeof(sd_pilot));
            sd_chr_from_trn(player1->chr, trn, player1->pilot);

            if(oldchr) {
                if(player1->pilot != &oldchr->pilot) {
                    sd_sprite_free(player1->pilot->photo);
                    omf_free(player1->pilot->photo);
                } else {
                    player1->pilot = NULL;
                }
                sd_chr_free(oldchr);
                omf_free(oldchr);
            }

            if(sg_save(player1->chr) != SD_SUCCESS) {
                log_error("Failed to save pilot %s", player1->chr->pilot.name);
            }
            // force the character to reload because its just easier

            sd_chr_free(player1->chr);
            omf_free(player1->chr);

            bool found = mechlab_find_last_player(scene);
            mechlab_select_dashboard(scene, DASHBOARD_STATS);
            gui_frame_free(local->frame);
            gui_theme theme;
            mechlab_theme(&theme);
            local->frame = gui_frame_create(&theme, 0, 0, 320, 200);
            component *menu = lab_menu_main_create(scene, found);
            gui_frame_set_root(local->frame, menu);
            gui_frame_layout(local->frame);
        } else {
            game_state_set_next(scene->gs, SCENE_MENU);
        }
    }
}

void mechlab_select_dashboard(scene *scene, dashboard_type type) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if(type == local->dashtype) {
        // No change
        return;
    }

    // Free old dashboard if set
    if(local->dashboard != NULL) {
        gui_frame_free(local->dashboard);
    }

    // Switch to new dashboard
    local->dashtype = type;
    gui_theme theme;
    mechlab_theme(&theme);
    switch(type) {
        // Dashboard with the gauges etc.
        case DASHBOARD_STATS:
        case DASHBOARD_SELECT_DIFFICULTY:
        case DASHBOARD_SELECT_NEW_PIC:
            // Dashboard widgets struct is filled with pointer to the necessary components for easy access
            local->dashboard = gui_frame_create(&theme, 0, 0, 320, 200);
            gui_frame_set_root(local->dashboard, lab_dash_main_create(scene, &local->dw));
            lab_dash_main_update(scene, &local->dw);
            gui_frame_layout(local->dashboard);
            break;
        case DASHBOARD_SIM:
            local->dashboard = gui_frame_create(&theme, 0, 0, 320, 200);
            gui_frame_set_root(local->dashboard, lab_dash_sim_create(scene, &local->dw));
            lab_dash_sim_update(scene, &local->dw, player1->pilot);
            gui_frame_layout(local->dashboard);
            break;
        // Dashboard for new player
        case DASHBOARD_NEW_PLAYER:
            if(player1->chr) {
                if(&player1->chr->pilot == player1->pilot) {
                    player1->pilot = omf_calloc(1, sizeof(sd_pilot));
                }
                sd_chr_free(player1->chr);
                omf_free(player1->chr);
            }
            local->dashboard = gui_frame_create(&theme, 0, 0, 320, 200);
            // new pilots have 2000 credits
            memset(player1->pilot, 0, sizeof(sd_pilot));
            player1->pilot->money = 2000;
            // and a jaguar
            player1->pilot->har_id = 0;
            // with no altpals yet
            player1->pilot->color_1 = 16;
            player1->pilot->color_2 = 16;
            player1->pilot->color_3 = 16;
            object_free(local->mech);
            omf_free(local->mech);
            local->mech = omf_calloc(1, sizeof(object));
            animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + player1->pilot->har_id)->ani;
            object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(local->mech, initial_har_ani);
            object_set_repeat(local->mech, 1);
            object_dynamic_tick(local->mech);

            gui_frame_set_root(local->dashboard, lab_dash_newplayer_create(scene, &local->nw));
            gui_frame_layout(local->dashboard);
            break;
        case DASHBOARD_SELECT_TOURNAMENT:
            local->dashboard = gui_frame_create(&theme, 0, 0, 320, 200);
            gui_frame_set_root(local->dashboard, lab_dash_trnselect_create(scene, &local->tw));
            gui_frame_layout(local->dashboard);
            break;
        // No dashboard selection. This shouldn't EVER happen.
        case DASHBOARD_NONE:
            log_error("No dashboard selected; this should not happen!");
            local->dashboard = NULL;
            break;
    }
}

int mechlab_event(scene *scene, SDL_Event *event) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);
    if(player1->ctrl->type == CTRL_TYPE_GAMEPAD ||
       (player1->ctrl->type == CTRL_TYPE_KEYBOARD && event->type == SDL_KEYDOWN &&
        keyboard_binds_key(player1->ctrl, event))) {
        // these events will be handled by polling
        return 1;
    }

    if(local->dashtype == DASHBOARD_STATS && event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_e) {
        // user is trying to use the cutscene replay cheat.
        if(!player1->chr || player1->chr->bk_name[0] == '\0' || player1->pilot->rank != 1) {
            log_info("Can't replay cutscene");
        } else {
            log_info("Replaying cutscene '%s'", player1->chr->bk_name);
            scene->gs->fight_stats.winner = -1;
            game_state_set_next(scene->gs, SCENE_TRN_CUTSCENE);
            return 1;
        }
    }

    if(local->dashtype == DASHBOARD_NEW_PLAYER) {
        return gui_frame_event(local->dashboard, event);
    } else {
        return gui_frame_event(local->frame, event);
    }
}

void mechlab_render(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(unsigned i = 0; i < N_ELEMENTS(local->bg_obj); i++) {
        if((local->dashtype == DASHBOARD_SELECT_TOURNAMENT || local->dashtype == DASHBOARD_SIM) && i > 0) {
            continue;
        }
        object_render(&local->bg_obj[i]);
    }

    // Render dashboard
    gui_frame_render(local->frame);

    if(local->dashtype != DASHBOARD_NEW_PLAYER && local->mech != NULL) {
        if(local->dashtype != DASHBOARD_SELECT_TOURNAMENT && local->dashtype != DASHBOARD_SIM) {
            object_render(local->mech);
        }
    }

    if(local->dashtype == DASHBOARD_STATS && local->mech != NULL) {
        gui_frame_render(local->dashboard);
    } else if(local->dashtype != DASHBOARD_STATS) {
        gui_frame_render(local->dashboard);
    }
    component_render(local->hint);

    if(local->popup) {
        video_draw_remap(&local->popup_bg1, (NATIVE_W - POPUP_BG_W) / 2, POPUP_CENTERY - POPUP_BG_H / 2, 4, 1, 0);
        video_draw(&local->popup_bg2, (NATIVE_W - POPUP_BG_W) / 2, POPUP_CENTERY - POPUP_BG_H / 2);
        text_draw(local->popup, (NATIVE_W - POPUP_TEXT_W) / 2, POPUP_CENTERY - POPUP_TEXT_H / 2);
    }
}

void mechlab_input_tick(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    // Poll the controller
    ctrl_event *p1 = NULL, *i;
    game_state_menu_poll(scene->gs, &p1);
    i = p1;
    if(i == NULL) {
        return;
    }
    do {
        if(i->type == EVENT_TYPE_ACTION) {
            if(local->popup) {
                if(i->event_data.action != ACT_STOP) {
                    text_free(&local->popup);
                }
                continue;
            }
            // If view is new dashboard view, pass all input to it
            else if(local->dashtype == DASHBOARD_NEW_PLAYER) {
                // If inputting text for new player name is done, switch to next view.
                // If ESC, exit view.
                // Otherwise handle text input
                if(i->event_data.action == ACT_ESC) {
                    bool found = mechlab_find_last_player(scene);
                    mechlab_select_dashboard(scene, DASHBOARD_STATS);
                    gui_frame_set_root(local->frame, lab_menu_main_create(scene, found));
                    gui_frame_layout(local->frame);
                } else if(i->event_data.action == ACT_PUNCH) {
                    if(strlen(textinput_value(local->nw.input)) > 0) {
                        strncpy(player1->pilot->name, textinput_value(local->nw.input), 17);
                        trnmenu_finish(
                            gui_frame_get_root(local->frame)); // This will trigger exception case in mechlab_tick
                    }
                } else {
                    log_debug("sending input %d to new player dash", i->event_data.action);
                    gui_frame_action(local->dashboard, i->event_data.action);
                }

            } else if(local->dashtype == DASHBOARD_SELECT_NEW_PIC && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                gui_frame_set_root(local->frame, lab_menu_main_create(scene, found));
                gui_frame_layout(local->frame);
            } else if(local->dashtype == DASHBOARD_SELECT_DIFFICULTY && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                gui_frame_set_root(local->frame, lab_menu_main_create(scene, found));
                gui_frame_layout(local->frame);
            } else if(local->dashtype == DASHBOARD_SELECT_TOURNAMENT && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                gui_frame_set_root(local->frame, lab_menu_main_create(scene, found));
                gui_frame_layout(local->frame);
            } else if(local->dashtype == DASHBOARD_SIM && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                gui_frame_set_root(local->frame, lab_menu_main_create(scene, found));
                gui_frame_layout(local->frame);
            } else {
                gui_frame_action(local->frame, i->event_data.action);
            }
        }
    } while((i = i->next) != NULL);
    controller_free_chain(p1);
}

// Init mechlab
int mechlab_create(scene *scene) {
    // Alloc
    mechlab_local *local = omf_calloc(1, sizeof(mechlab_local));
    local->selling = false;

    // Default theme for mechlab
    mechlab_theme(&local->theme);

    // reset the match settings and override the settings that don't apply in tournament mode
    game_state_match_settings_reset(scene->gs);
    scene->gs->match_settings.power1 = 5;
    scene->gs->match_settings.power2 = 5;
    scene->gs->match_settings.vitality = 100;
    scene->gs->match_settings.rounds = 0;

    // so 'SCENE TRN_CUTSCENE' console cheat skips the VS screen.
    scene->gs->fight_stats.winner = -1;

    animation *bg_ani[3];

    // Init the background
    for(unsigned i = 0; i < N_ELEMENTS(bg_ani); i++) {
        sprite *spr = sprite_copy(animation_get_sprite(&bk_get_info(scene->bk_data, 14)->ani, i));
        bg_ani[i] = create_animation_from_single(spr, spr->pos);
        object_create(&local->bg_obj[i], scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(&local->bg_obj[i], bg_ani[i]);
        object_select_sprite(&local->bg_obj[i], 0);
        object_set_repeat(&local->bg_obj[i], 1);
        object_set_animation_owner(&local->bg_obj[i], OWNER_OBJECT);
    }

    local->hint = label_create("HINTY");
    label_set_font(local->hint, FONT_SMALL);
    label_set_text_horizontal_align(local->hint, TEXT_ALIGN_CENTER);
    label_set_text_vertical_align(local->hint, TEXT_ALIGN_MIDDLE);
    label_set_text_color(local->hint, MECHLAB_YELLOW);
    component_set_pos_hints(local->hint, 32, 131);
    component_set_size_hints(local->hint, 248, 13);
    component_init(local->hint, &local->theme);
    component_layout(local->hint, 32, 131, 248, 13);

    scene_set_userdata(scene, local);
    bool found = mechlab_find_last_player(scene);
    mechlab_select_dashboard(scene, DASHBOARD_STATS);

    menu_transparent_bg_create(&local->popup_bg1, POPUP_BG_W, POPUP_BG_H);
    menu_background_create(&local->popup_bg2, POPUP_BG_W, POPUP_BG_H, MenuBackgroundNewsroom);

    // Create main menu
    local->frame = gui_frame_create(&local->theme, 0, 0, 320, 200);
    component *menu = lab_menu_main_create(scene, found);
    gui_frame_set_root(local->frame, menu);
    gui_frame_layout(local->frame);

    // Set callbacks
    scene_set_input_poll_cb(scene, mechlab_input_tick);
    scene_set_event_cb(scene, mechlab_event);
    scene_set_render_cb(scene, mechlab_render);
    scene_set_free_cb(scene, mechlab_free);
    scene_set_dynamic_tick_cb(scene, mechlab_tick);

    return 0;
}
