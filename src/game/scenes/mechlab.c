#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "formats/error.h"
#include "formats/tournament.h"
#include "game/game_state.h"
#include "game/gui/frame.h"
#include "game/gui/label.h"
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
#include "resources/pathmanager.h"
#include "resources/sgmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/video.h"

typedef struct {
    dashboard_type dashtype;
    object bg_obj[3];
    guiframe *frame;
    guiframe *dashboard;
    object *mech;
    dashboard_widgets dw;
    newplayer_widgets nw;
    trnselect_widgets tw;
    bool selling;
    component *hint;
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
            PERROR("Could not load saved game for playername '%s': %s!", last_name, sd_get_error(ret));
            last_name = NULL;
        } else {
            p1->chr = chr;
            DEBUG("Loaded savegame for playername '%s'.", last_name);
        }
    }

    // Either initialize a new tournament if no savegame is found,
    // or just show old savegame stats directly if it was.
    local->dashtype = DASHBOARD_NONE;
    if(p1->chr == NULL) {
        DEBUG("No previous savegame found");
        object_free(local->mech);
        omf_free(local->mech);
        p1->pilot->money = 0;
        p1->pilot->har_id = 0;
        return false;
    } else {
        DEBUG("Previous savegame found; loading as default.");
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
            omf_free(old_pilot);
        }
        game_player_set_pilot(p1, &p1->chr->pilot);
    }
    return true;
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
    if(strlen(player1->pilot->name) != 0) {
        DEBUG("saving player %s", player1->pilot->name);
        char tmp[1024];
        const char *dirname = pm_get_local_path(SAVE_PATH);
        snprintf(tmp, 1024, "%s%s.CHR", dirname, player1->pilot->name);
        sd_chr_save(player1->chr, tmp);
        omf_free(settings_get()->tournament.last_name);
        settings_get()->tournament.last_name = strdup(player1->pilot->name);
        settings_save();
    } else {
        DEBUG("not saving pilot");
    }

    for(int i = 0; i < sizeof(local->bg_obj) / sizeof(object); i++) {
        object_free(&local->bg_obj[i]);
    }

    component_free(local->hint);
    guiframe_free(local->frame);
    guiframe_free(local->dashboard);
    object_free(local->mech);
    omf_free(local->mech);
    omf_free(local);
    scene_set_userdata(scene, local);
}

void mechlab_enter_trnselect_menu(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_trnselect_select, &local->tw, lab_dash_trnselect_left,
                                             &local->tw, lab_dash_trnselect_right, &local->tw, 486, true);
    guiframe_set_root(local->frame, menu);
    guiframe_layout(local->frame);
}

component *mechlab_chrload_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_load, &local->dw, lab_dash_main_chr_left,
                                             &local->dw, lab_dash_main_chr_right, &local->dw, 225, true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_main_chr_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_main_chr_done);
    trnmenu_set_userdata(menu, &local->dw);
    return menu;
}

component *mechlab_chrdelete_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_delete, &local->dw, lab_dash_main_chr_left,
                                             &local->dw, lab_dash_main_chr_right, &local->dw, 226, true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_main_chr_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_main_chr_done);
    trnmenu_set_userdata(menu, &local->dw);
    return menu;
}

component *mechlab_sim_menu_create(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    component *menu = lab_menu_select_create(scene, lab_dash_main_chr_delete, &local->dw, lab_dash_sim_left, &local->dw,
                                             lab_dash_sim_right, &local->dw, 227, true);
    trnmenu_set_submenu_init_cb(menu, lab_dash_sim_init);
    trnmenu_set_submenu_done_cb(menu, lab_dash_sim_done);
    trnmenu_set_userdata(menu, &local->dw);
    return menu;
}

void mechlab_update(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *p1 = game_state_get_player(scene->gs, 0);
    animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + p1->pilot->har_id)->ani;
    if(local->mech && object_get_animation(local->mech) != initial_har_ani) {
        object_free(local->mech);
        object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(local->mech, initial_har_ani);
        object_set_repeat(local->mech, 1);
        object_dynamic_tick(local->mech);
    }

    lab_dash_main_update(scene, &local->dw);
}

void mechlab_tick(scene *scene, int paused) {
    mechlab_local *local = scene_get_userdata(scene);

    guiframe_tick(local->frame);
    guiframe_tick(local->dashboard);
    if(local->mech != NULL) {
        object_dynamic_tick(local->mech);
    }

    // Check if root is finished
    component *root = guiframe_get_root(local->frame);
    if(trnmenu_is_finished(root)) {
        if(local->dashtype == DASHBOARD_NEW) {
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_NEW_PIC);
            guiframe_free(local->frame);
            local->frame = guiframe_create(0, 0, 320, 200);
            component *menu =
                lab_menu_select_create(scene, lab_dash_main_photo_select, &local->dw, lab_dash_main_photo_left,
                                       &local->dw, lab_dash_main_photo_right, &local->dw, 224, true);
            guiframe_set_root(local->frame, menu);
            guiframe_layout(local->frame);
        } else if(local->dashtype == DASHBOARD_SELECT_NEW_PIC) {
            // game_player *player1 = game_state_get_player(scene->gs, 0);
            // player1->pilot->photo_id =  lab_dash_main_pilotselected(&local->dw);
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_DIFFICULTY);
            guiframe_free(local->frame);
            local->frame = guiframe_create(0, 0, 320, 200);
            component *menu = lab_menu_difficultyselect_create(scene);
            // trnmenu_attach(menu, local->hint);
            guiframe_set_root(local->frame, menu);
            guiframe_layout(local->frame);
        } else if(local->dashtype == DASHBOARD_SELECT_DIFFICULTY) {
            mechlab_select_dashboard(scene, DASHBOARD_SELECT_TOURNAMENT);
            guiframe_free(local->frame);
            local->frame = guiframe_create(0, 0, 320, 200);
            mechlab_enter_trnselect_menu(scene);
        } else if(local->dashtype == DASHBOARD_SELECT_TOURNAMENT) {
            sd_tournament_file *trn = lab_dash_trnselect_selected(&local->tw);
            game_player *player1 = game_state_get_player(scene->gs, 0);
            if(player1->pilot->money < trn->registration_fee) {
                player1->pilot->money = 0;
            } else {
                player1->pilot->money = player1->pilot->money - trn->registration_fee;
            }
            player1->chr = omf_calloc(1, sizeof(sd_chr_file));
            memcpy(&player1->chr->pilot, player1->pilot, sizeof(sd_pilot));
            sd_chr_from_trn(player1->chr, trn, player1->pilot);
            char tmp[1024];
            const char *dirname = pm_get_local_path(SAVE_PATH);
            snprintf(tmp, 1024, "%s%s.CHR", dirname, player1->pilot->name);
            sd_chr_save(player1->chr, tmp);
            omf_free(settings_get()->tournament.last_name);
            settings_get()->tournament.last_name = strdup(player1->pilot->name);
            settings_save();
            // force the character to reload because its just easier
            sd_chr_free(player1->chr);
            omf_free(player1->chr);
            bool found = mechlab_find_last_player(scene);
            mechlab_select_dashboard(scene, DASHBOARD_STATS);
            guiframe_free(local->frame);
            local->frame = guiframe_create(0, 0, 320, 200);
            component *menu = lab_menu_main_create(scene, found);
            // trnmenu_attach(menu, local->hint);
            guiframe_set_root(local->frame, menu);
            guiframe_layout(local->frame);
        } else {
            game_player *player1 = game_state_get_player(scene->gs, 0);
            if(player1->chr) {
                sd_chr_free(player1->chr);
                omf_free(player1->chr);
                player1->pilot = omf_calloc(1, sizeof(sd_pilot));
                sd_pilot_create(player1->pilot);
            }
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
        guiframe_free(local->dashboard);
    }

    // Switch to new dashboard
    local->dashtype = type;
    switch(type) {
        // Dashboard with the gauges etc.
        case DASHBOARD_STATS:
        case DASHBOARD_SELECT_DIFFICULTY:
        case DASHBOARD_SELECT_NEW_PIC:
            // Dashboard widgets struct is filled with pointer to the necessary components for easy access
            local->dashboard = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->dashboard, lab_dash_main_create(scene, &local->dw));
            lab_dash_main_update(scene, &local->dw);
            guiframe_layout(local->dashboard);
            break;
        case DASHBOARD_SIM:
            local->dashboard = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->dashboard, lab_dash_sim_create(scene, &local->dw));
            lab_dash_sim_update(scene, &local->dw, player1->pilot);
            guiframe_layout(local->dashboard);
            break;
        // Dashboard for new player
        case DASHBOARD_NEW:
            local->dashboard = guiframe_create(0, 0, 320, 200);
            // new pilots have 2000 credits
            memset(player1->pilot, 0, sizeof(sd_pilot));
            player1->pilot->money = 2000;
            // and a jaguar
            player1->pilot->har_id = 0;
            object_free(local->mech);
            omf_free(local->mech);
            local->mech = omf_calloc(1, sizeof(object));
            animation *initial_har_ani = &bk_get_info(scene->bk_data, 15 + player1->pilot->har_id)->ani;
            object_create(local->mech, scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
            object_set_animation(local->mech, initial_har_ani);
            object_set_repeat(local->mech, 1);
            object_dynamic_tick(local->mech);

            guiframe_set_root(local->dashboard, lab_dash_newplayer_create(scene, &local->nw));
            guiframe_layout(local->dashboard);
            break;
        case DASHBOARD_SELECT_TOURNAMENT:
            local->dashboard = guiframe_create(0, 0, 320, 200);
            guiframe_set_root(local->dashboard, lab_dash_trnselect_create(scene, &local->tw));
            guiframe_layout(local->dashboard);
            break;
        // No dashboard selection. This shouldn't EVER happen.
        case DASHBOARD_NONE:
            PERROR("No dashboard selected; this should not happen!");
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

    if(local->dashtype == DASHBOARD_NEW) {
        return guiframe_event(local->dashboard, event);
    } else {
        return guiframe_event(local->frame, event);
    }
}

void mechlab_render(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);

    for(int i = 0; i < sizeof(local->bg_obj) / sizeof(object); i++) {
        if(local->dashtype == DASHBOARD_SELECT_TOURNAMENT && i > 0) {
            continue;
        }
        object_render(&local->bg_obj[i]);
    }

    // Render dashboard
    guiframe_render(local->frame);

    if(local->dashtype != DASHBOARD_NEW && local->mech != NULL) {
        if(local->dashtype != DASHBOARD_SELECT_TOURNAMENT) {
            object_render(local->mech);
        }
    }

    if(local->dashtype == DASHBOARD_STATS && local->mech != NULL) {
        guiframe_render(local->dashboard);
    } else if(local->dashtype != DASHBOARD_STATS) {
        guiframe_render(local->dashboard);
    }
    component_render(local->hint);
}

void mechlab_input_tick(scene *scene) {
    mechlab_local *local = scene_get_userdata(scene);
    game_player *player1 = game_state_get_player(scene->gs, 0);

    // Poll the controller
    ctrl_event *p1 = NULL, *i;
    controller_poll(player1->ctrl, &p1);
    i = p1;
    if(i == NULL) {
        return;
    }
    do {
        if(i->type == EVENT_TYPE_ACTION) {
            // If view is new dashboard view, pass all input to it
            if(local->dashtype == DASHBOARD_NEW) {
                // If inputting text for new player name is done, switch to next view.
                // If ESC, exit view.
                // Otherwise handle text input
                if(i->event_data.action == ACT_ESC) {
                    mechlab_find_last_player(scene);
                    mechlab_select_dashboard(scene, DASHBOARD_STATS);
                } else if(i->event_data.action == ACT_KICK || i->event_data.action == ACT_PUNCH) {
                    if(strlen(textinput_value(local->nw.input)) > 0) {
                        strncpy(player1->pilot->name, textinput_value(local->nw.input), 17);
                        trnmenu_finish(
                            guiframe_get_root(local->frame)); // This will trigger exception case in mechlab_tick
                    }
                } else {
                    guiframe_action(local->dashboard, i->event_data.action);
                }

            } else if(local->dashtype == DASHBOARD_SELECT_NEW_PIC && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                guiframe_set_root(local->frame, lab_menu_main_create(scene, found));
                guiframe_layout(local->frame);
            } else if(local->dashtype == DASHBOARD_SELECT_DIFFICULTY && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                guiframe_set_root(local->frame, lab_menu_main_create(scene, found));
                guiframe_layout(local->frame);
            } else if(local->dashtype == DASHBOARD_SELECT_TOURNAMENT && i->event_data.action == ACT_ESC) {
                bool found = mechlab_find_last_player(scene);
                mechlab_select_dashboard(scene, DASHBOARD_STATS);
                guiframe_set_root(local->frame, lab_menu_main_create(scene, found));
                guiframe_layout(local->frame);
            } else {
                guiframe_action(local->frame, i->event_data.action);
            }
        }
    } while((i = i->next));
    controller_free_chain(p1);
}

// Init mechlab
int mechlab_create(scene *scene) {
    // Alloc
    mechlab_local *local = omf_calloc(1, sizeof(mechlab_local));
    local->selling = false;

    animation *bg_ani[3];

    // Init the background
    for(int i = 0; i < sizeof(bg_ani) / sizeof(animation *); i++) {
        sprite *spr = sprite_copy(animation_get_sprite(&bk_get_info(scene->bk_data, 14)->ani, i));
        bg_ani[i] = create_animation_from_single(spr, spr->pos);
        object_create(&local->bg_obj[i], scene->gs, vec2i_create(0, 0), vec2f_create(0, 0));
        object_set_animation(&local->bg_obj[i], bg_ani[i]);
        object_select_sprite(&local->bg_obj[i], 0);
        object_set_repeat(&local->bg_obj[i], 1);
        object_set_animation_owner(&local->bg_obj[i], OWNER_OBJECT);
    }

    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_SMALL;
    tconf.halign = TEXT_CENTER;
    tconf.valign = TEXT_MIDDLE;
    tconf.cforeground = color_create(255, 255, 0, 255);

    local->hint = label_create(&tconf, "HINTY");
    component_set_pos_hints(local->hint, 32, 131);
    component_set_size_hints(local->hint, 248, 13);
    component_layout(local->hint, 32, 131, 248, 13);

    scene_set_userdata(scene, local);
    bool found = mechlab_find_last_player(scene);
    mechlab_select_dashboard(scene, DASHBOARD_STATS);

    // Create main menu
    local->frame = guiframe_create(0, 0, 320, 200);
    component *menu = lab_menu_main_create(scene, found);
    // trnmenu_attach(menu, local->hint);
    guiframe_set_root(local->frame, menu);
    guiframe_layout(local->frame);

    // Set callbacks
    scene_set_input_poll_cb(scene, mechlab_input_tick);
    scene_set_event_cb(scene, mechlab_event);
    scene_set_render_cb(scene, mechlab_render);
    scene_set_free_cb(scene, mechlab_free);
    scene_set_dynamic_tick_cb(scene, mechlab_tick);

    return 0;
}
