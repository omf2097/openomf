#include <enet/enet.h>
#include <time.h>

#include "game/scenes/mainmenu/menu_listen.h"

#include "game/game_state.h"
#include "game/gui/gui.h"
#include "game/protos/scene.h"
#include "game/utils/nat.h"
#include "game/utils/settings.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct {
    ENetHost *host;
    int controllers_created;
    component *cancel_button;
    nat_ctx nat;
    scene *s;
} listen_menu_data;

void menu_listen_free(component *c) {
    listen_menu_data *local = menu_get_userdata(c);
    if(local->host && !local->controllers_created) {
        enet_host_destroy(local->host);
    }
    local->host = NULL;
    local->controllers_created = 0;
    nat_free(&local->nat);
    omf_free(local);
    menu_set_userdata(c, local);
}

void menu_listen_tick(component *c) {
    listen_menu_data *local = menu_get_userdata(c);
    game_state *gs = local->s->gs;
    if(local->host) {
        ENetEvent event;
        while(!local->controllers_created && enet_host_service(local->host, &event, 0) > 0) {
            if(event.type != ENET_EVENT_TYPE_CONNECT) {
                continue;
            }

            ENetPacket *packet = enet_packet_create("0", 2, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(local->host);

            log_debug("client connected!");
            controller *player1_ctrl, *player2_ctrl;
            keyboard_keys *keys;
            game_player *p1 = game_state_get_player(gs, 0);
            game_player *p2 = game_state_get_player(gs, 1);

            // force the speed to 3
            game_state_set_speed(gs, 10);

            p1->pilot->har_id = HAR_JAGUAR;
            p1->pilot->pilot_id = 0;
            p2->pilot->har_id = HAR_JAGUAR;
            p2->pilot->pilot_id = 0;

            player1_ctrl = omf_calloc(1, sizeof(controller));
            controller_init(player1_ctrl, gs);
            player1_ctrl->har_obj_id = p1->har_obj_id;
            player2_ctrl = omf_calloc(1, sizeof(controller));
            controller_init(player2_ctrl, gs);
            player2_ctrl->har_obj_id = p2->har_obj_id;

            // Player 1 controller -- Keyboard
            settings_keyboard *k = &settings_get()->keys;
            keys = omf_calloc(1, sizeof(keyboard_keys));
            keys->jump_up = SDL_GetScancodeFromName(k->key1_jump_up);
            keys->jump_right = SDL_GetScancodeFromName(k->key1_jump_right);
            keys->walk_right = SDL_GetScancodeFromName(k->key1_walk_right);
            keys->duck_forward = SDL_GetScancodeFromName(k->key1_duck_forward);
            keys->duck = SDL_GetScancodeFromName(k->key1_duck);
            keys->duck_back = SDL_GetScancodeFromName(k->key1_duck_back);
            keys->walk_back = SDL_GetScancodeFromName(k->key1_walk_back);
            keys->jump_left = SDL_GetScancodeFromName(k->key1_jump_left);
            keys->punch = SDL_GetScancodeFromName(k->key1_punch);
            keys->kick = SDL_GetScancodeFromName(k->key1_kick);
            keyboard_create(player1_ctrl, keys, 0);
            game_player_set_ctrl(p1, player1_ctrl);

            // Player 2 controller -- Network
            net_controller_create(player2_ctrl, local->host, event.peer, NULL, ROLE_SERVER);
            game_player_set_ctrl(p2, player2_ctrl);
            game_player_set_selectable(p2, 1);

            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)), AI_DIFFICULTY_CHAMPION);
            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)), AI_DIFFICULTY_CHAMPION);

            local->controllers_created = 1;
            break;
        }
        game_player *p2 = game_state_get_player(gs, 1);
        controller *c2 = game_player_get_ctrl(p2);
        if(c2->type == CTRL_TYPE_NETWORK && net_controller_ready(c2) == 1) {
            log_debug("network peer is ready, tick offset is %d and rtt is %d", net_controller_tick_offset(c2),
                      c2->rtt);
            local->host = NULL;
            local->controllers_created = 0;
            game_state_set_next(gs, SCENE_MELEE);
        }
    }
}

void menu_listen_cancel(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    // Clean up host
    listen_menu_data *local = menu_get_userdata(c->parent);
    if(local->host && !local->controllers_created) {
        enet_host_destroy(local->host);
    }
    local->controllers_created = 0;
    local->host = NULL;
    nat_free(&local->nat);
}

component *menu_listen_create(scene *s) {
    listen_menu_data *local = omf_calloc(1, sizeof(listen_menu_data));
    s->gs->role = ROLE_SERVER;
    local->s = s;

    // Form address (host)
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = settings_get()->net.net_listen_port_start;

    if(address.port == 0) {
        address.port = rand_int(65535 - 1024) + 1024;
    }

    // Set up host
    local->controllers_created = 0;
    int randtries = 0;
    int end_port = settings_get()->net.net_listen_port_end;
    if(!end_port) {
        end_port = 65535;
    }
    while(local->host == NULL) {
        local->host = enet_host_create(&address, 1, 3, 0, 0);
        if(local->host == NULL) {
            if(settings_get()->net.net_listen_port_start == 0) {
                address.port = rand_int(65535 - 1024) + 1024;
                randtries++;
                if(randtries > 10) {
                    log_debug("Failed to initialize ENet server with random ports");
                    omf_free(local);
                    return NULL;
                }
            } else {
                address.port++;
                if(address.port > end_port || randtries > 10) {
                    log_debug("Failed to initialize ENet server between ports %d and %d after 10 attempts",
                              settings_get()->net.net_listen_port_start, end_port);
                    omf_free(local);
                    return NULL;
                }
            }
        }
    }
    nat_create(&local->nat);

    int ext_port = 0;

    if(local->nat.type != NAT_TYPE_NONE) {
        ext_port = settings_get()->net.net_ext_port_start;
        if(ext_port == 0) {
            // try to use the internal port first
            ext_port = address.port;
        }
        randtries = 0;
        while(!nat_create_mapping(&local->nat, address.port, ext_port)) {
            if(settings_get()->net.net_ext_port_start == 0) {
                ext_port = rand_int(65535 - 1024) + 1024;
                randtries++;
                if(randtries > 10) {
                    ext_port = 0;
                    break;
                }
            } else {
                ext_port++;
                if(settings_get()->net.net_ext_port_end && ext_port > settings_get()->net.net_ext_port_end) {
                    ext_port = 0;
                    break;
                }
            }
        }
    }
    enet_socket_set_option(local->host->socket, ENET_SOCKOPT_REUSEADDR, 1);

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = TEXT_BRIGHT_GREEN;

    // Create the menu
    component *menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "START SERVER"));
    menu_attach(menu, filler_create());
    char buf[200];
    if(local->nat.type != NAT_TYPE_NONE) {
        snprintf(buf, sizeof(buf), "Waiting for local connections to port %d and external connections to %s port %d.",
                 address.port, local->nat.wan_address, local->nat.ext_port);
    } else {
        snprintf(buf, sizeof(buf), "Waiting for connections to port %d.", ext_port ? ext_port : address.port);
    }
    menu_attach(menu, label_create(&tconf, buf));
    menu_attach(menu, filler_create());
    local->cancel_button =
        textbutton_create(&tconf, "CANCEL", "Cancel listing for a connection.", COM_ENABLED, menu_listen_cancel, s);
    menu_attach(menu, local->cancel_button);

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_listen_free);
    menu_set_tick_cb(menu, menu_listen_tick);
    return menu;
}
