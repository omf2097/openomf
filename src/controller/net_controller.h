#ifndef NET_CONTROLLER_H
#define NET_CONTROLLER_H

#define NET_INPUT_DELAY 2
#define MAX_EVENTS_PER_TICK 11

#include "controller/controller.h"
#include "utils/list.h"
#include "game/game_state.h"
#include <SDL.h>
#include <enet/enet.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t tick;
    uint8_t events[2][MAX_EVENTS_PER_TICK];
} tick_events;

typedef struct {
    ENetHost *host;
    ENetPeer *peer;
    ENetPeer *lobby;
    int id;
    uint32_t last_hb;
    uint32_t outstanding_hb;
    int disconnected;
    int rttbuf[100];
    int rttpos;
    int rttfilled;
    int tick_offset;
    int frame_advantage;
    bool synchronized;
    int guesses;
    uint32_t peer_proposal;
    uint32_t local_proposal;
    bool confirmed;
    uint32_t last_tick;
    uint32_t last_sent_tick;
    list transcript;
    uint32_t last_received_tick;
    uint32_t last_acked_tick;
    int last_har_state;
    uint32_t last_traced_tick;
    uint32_t peer_last_hash;
    uint32_t peer_last_hash_tick;
    uint32_t last_hash;
    uint32_t last_hash_tick;
    uint8_t last_action;
    uint32_t last_rewind_tick;
    uint32_t last_peer_input_tick;
    uint8_t last_peer_action;
    SDL_RWops *trace_file;
    game_state *gs_bak;
    int winner;
} wtf;

void insert_event(wtf *data, uint32_t tick, uint16_t action, int id);

void net_controller_create(controller *ctrl, ENetHost *host, ENetPeer *peer, ENetPeer *lobby, int id);
void net_controller_free(controller *ctrl);
int net_controller_get_rtt(controller *ctrl);
void net_controller_har_hook(int action, void *cb_data);

bool net_controller_ready(controller *ctrl);
int net_controller_tick_offset(controller *ctrl);

ENetPeer *net_controller_get_lobby_connection(controller *ctrl);

ENetHost *net_controller_get_host(controller *ctrl);
int net_controller_get_winner(controller *ctrl);
void net_controller_set_winner(controller *ctrl, int winner);

void menu_controller_hook(controller *ctrl, int action);

#endif // NET_CONTROLLER_H
