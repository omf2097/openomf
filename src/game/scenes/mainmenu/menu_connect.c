#include <enet/enet.h>
#include <time.h>

#include "game/scenes/mainmenu/menu_connect.h"
#include "game/scenes/mainmenu/menu_widget_ids.h"

#include "game/gui/gui.h"
#include "game/utils/settings.h"
#include "game/protos/scene.h"
#include "game/game_state.h"
#include "utils/compat.h"
#include "utils/log.h"

typedef struct {
    time_t connect_start;
    ENetHost *host;
    component *addr_input;
    component *connect_button;
    component *cancel_button;
    scene *s;
} connect_menu_data;

void menu_connect_free(component *c) {
    connect_menu_data *local = menu_get_userdata(c);
    if(local->host) {
        enet_host_destroy(local->host);
    }
    free(local);
}

void menu_connect_start(component *c, void *userdata) {
    scene *s = userdata;
    connect_menu_data *local = menu_get_userdata(c->parent);
    ENetAddress address;
    const char *addr = textinput_value(local->addr_input);
    s->gs->role = ROLE_CLIENT;

    // Free old saved address, and set new
    free(settings_get()->net.net_connect_ip);
    settings_get()->net.net_connect_ip = strdup(addr);

    // Set up enet host
    local->host = enet_host_create(NULL, 1, 2, 0, 0);
    if(local->host == NULL) {
        DEBUG("Failed to initialize ENet client");
        return;
    }

    // Disable connect button and address input field
    component_disable(local->connect_button, 1);
    component_disable(local->addr_input, 1);
    menu_select(c->parent, local->cancel_button);

    // Set address
    enet_address_set_host(&address, addr);
    address.port = settings_get()->net.net_connect_port;

    ENetPeer *peer = enet_host_connect(local->host, &address, 2, 0);
    if(peer == NULL) {
        DEBUG("Unable to connect to %s", addr);
        enet_host_destroy(local->host);
        local->host = NULL;
    }
    time(&local->connect_start);
}

void menu_connect_cancel(component *c, void *userdata) {
    menu *m = sizer_get_obj(c->parent);
    m->finished = 1;

    // Clean up host
    connect_menu_data *local = menu_get_userdata(c->parent);
    if(local->host) {
        enet_host_destroy(local->host);
        local->host = NULL;
    }
}

void menu_connect_tick(component *c) {
    connect_menu_data *local = menu_get_userdata(c);
    game_state *gs = local->s->gs;
    if(local->host) {
        ENetEvent event;
        if(enet_host_service(local->host, &event, 0) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
            ENetPacket * packet = enet_packet_create("0", 2, ENET_PACKET_FLAG_RELIABLE);
            enet_peer_send(event.peer, 0, packet);
            enet_host_flush(local->host);

            DEBUG("connected to server!");
            controller *player1_ctrl, *player2_ctrl;
            keyboard_keys *keys;
            game_player *p1 = game_state_get_player(gs, 0);
            game_player *p2 = game_state_get_player(gs, 1);

            // force the speed to 3
            game_state_set_speed(gs, 5);

            p1->har_id = HAR_JAGUAR;
            p1->pilot_id = 0;
            p2->har_id = HAR_JAGUAR;
            p2->pilot_id = 0;

            player1_ctrl = malloc(sizeof(controller));
            controller_init(player1_ctrl);
            player1_ctrl->har = p1->har;
            player2_ctrl = malloc(sizeof(controller));
            controller_init(player2_ctrl);
            player2_ctrl->har = p2->har;

            // Player 1 controller -- Network
            net_controller_create(player1_ctrl, local->host, event.peer, ROLE_CLIENT);
            game_player_set_ctrl(p1, player1_ctrl);

            // Player 2 controller -- Keyboard
            settings_keyboard *k = &settings_get()->keys;
            keys = malloc(sizeof(keyboard_keys));
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
            keys->escape = SDL_GetScancodeFromName(k->key1_escape);
            keyboard_create(player2_ctrl, keys, 0);
            game_player_set_ctrl(p2, player2_ctrl);
            local->host = NULL;
            game_player_set_selectable(p2, 1);

            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 0)), AI_DIFFICULTY_CHAMPION);
            chr_score_set_difficulty(game_player_get_score(game_state_get_player(gs, 1)), AI_DIFFICULTY_CHAMPION);

            game_state_set_next(gs, SCENE_MELEE);
        } else {
            if(difftime(time(NULL), local->connect_start) > 5.0) {
                DEBUG("connection timed out");
                menu_connect_cancel(local->cancel_button, local->s);
            }
        }
    }
}

component* menu_connect_create(scene *s) {
    connect_menu_data *local = malloc(sizeof(connect_menu_data));
    memset(local, 0, sizeof(connect_menu_data));
    local->s = s;

    // Text config
    text_settings tconf;
    text_defaults(&tconf);
    tconf.font = FONT_BIG;
    tconf.halign = TEXT_CENTER;
    tconf.cforeground = color_create(0, 121, 0, 255);

    component* menu = menu_create(11);
    menu_attach(menu, label_create(&tconf, "CONNECT TO SERVER"));
    menu_attach(menu, filler_create());

    local->addr_input = textinput_create(&tconf, "Host/IP", settings_get()->net.net_connect_ip);
    local->connect_button = textbutton_create(&tconf, "CONNECT", COM_ENABLED, menu_connect_start, s);
    local->cancel_button = textbutton_create(&tconf, "CANCEL", COM_ENABLED, menu_connect_cancel, s);
    widget_set_id(local->connect_button, NETWORK_CONNECT_IP_BUTTON_ID);
    menu_attach(menu, local->addr_input);
    menu_attach(menu, local->connect_button);
    menu_attach(menu, local->cancel_button);

    menu_set_userdata(menu, local);
    menu_set_free_cb(menu, menu_connect_free);
    menu_set_tick_cb(menu, menu_connect_tick);

    return menu;
}
