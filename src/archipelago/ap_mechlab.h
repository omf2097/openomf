#ifndef AP_MECHLAB_H
#define AP_MECHLAB_H

#include <stdbool.h>
#include <stdint.h>
#include "formats/error.h"
#include "formats/pilot.h"
#include "formats/tournament.h"
#include "game/gui/component.h"
#include "game/protos/scene.h"
#include "utils/vector.h"

typedef struct game_player_t game_player;
typedef struct game_state_t game_state;

// Expose ap_mode so files only need to include ap_mechlab.h for AP gating.
extern bool ap_mode;

// Mechlab lifecycle: call attach on mechlab_create, detach on mechlab_free.
void ap_mechlab_attach(scene *s);
void ap_mechlab_detach(void);

// Find AP pilot from save ident; set pilot + mech. Then attach mechlab callbacks.
bool ap_mechlab_find_player(scene *s);
bool ap_mechlab_find_and_attach(scene *s);

// Persist AP state and CHR for a game_player.
void ap_mechlab_save(game_player *p1);

// Map tournament filename to AP tournament index and match offset.
void ap_mechlab_set_tournament(sd_tournament_file *trn);

// --- Arena helpers ---

// Send AP checks for a tournament match win.
void ap_arena_match_win(game_state *gs, game_player *p1, game_player *p2);

// --- Trade menu helpers ---

// Preview a HAR in the trade menu (set har_id, sync stats, update display).
void ap_preview_har(scene *s, int har_id);

// Full AP confirm-trade handler — replaces the vanilla money/stats swap.
void ap_confirm_trade(component *c, scene *s, game_player *p1);

// --- Training (lab_menu_training.c) ---

// Price table lookup — use instead of a local prices[] in training UI.
int32_t ap_train_price(int level);

// Register the two label components from lab_menu_training_create.
// Cleared automatically on ap_mechlab_detach.
void ap_register_train_labels(component *stat_label, component *price_label);

// Update training labels after a buy or on focus.
// next_level: the level now being offered (current buy_level for focus, buy_level+1 after buy).
void ap_update_train_labels(int stat_lang_id, int next_level);

// Send a training check and pre-scout the following location.
void ap_do_train(scene *s, int stat);
// Scout the current-level training location for the hint text.
void ap_focus_train(scene *s, int stat);

// Full AP training buy, price-check, and focus handlers.
void ap_training_buy(scene *s, int stat);
void ap_training_check_price(component *c, scene *s, int stat);
void ap_training_focus(scene *s, int stat);

// --- HAR upgrades (lab_menu_customize.c) ---

// ap_buy_price is also used inline by focus handlers for price display.
int32_t ap_buy_price(int32_t vanilla_price, int current_level);

// Register the two label components from lab_menu_customize_create.
// Cleared automatically on ap_mechlab_detach.
void ap_register_buy_labels(component *header_label, component *details_label);

// Update HAR buy labels after a buy or on focus.
// next_buy_level: the level now being offered (APChecks level for focus, APChecks+1 after buy).
void ap_update_buy_har_labels(sd_pilot *pilot, int stat, int next_buy_level);

void ap_do_buy_har(scene *s, int har_id, int stat);
void ap_focus_buy_har(scene *s, int har_id, int stat);

// Full AP HAR buy, price-check, and focus handlers.
void ap_customize_buy(scene *s, sd_pilot *pilot, int stat);
void ap_customize_check_price(component *c, sd_pilot *pilot, int stat);
void ap_customize_focus(scene *s, sd_pilot *pilot, int stat);

bool ap_has_har_color_primary(void);
bool ap_has_har_color_secondary(void);
bool ap_has_har_color_tertiary(void);

// --- Other ---

// Apply AP progressive HAR/pilot stats to a pilot (e.g. after a HAR trade).
void ap_apply_har_stats(sd_pilot *pilot);

// Full AP trade confirm handler: builds message, creates confirm menu, finishes parent.
void ap_do_trade(component *c, scene *s);

// --- Trade menu (lab_menu_trade.c) ---

// Build the display slice for the trade menu: eligible HARs sorted by ID,
// excluding current_har_id, showing up to 6 at a time and cycling on each call.
// display_out must hold at least 6 ints. Returns the number of HARs written.
int ap_trade_page(int current_har_id, uint16_t har_trades, int *display_out);

// Match / tournament check sends (arena.c, newsroom.c).
void ap_on_match_win(int trn_index);
void ap_on_tournament_win(void);

// Filter trnlist to only include tournaments accessible by the player's
// Progressive Tournament Access count. Call after trnlist_init in AP mode.
void ap_filter_trnlist(vector *trnlist);

// mechlab_set_hint with word-wrap at 39 characters.
void ap_set_hint(scene *s, const char *hint);

#endif // AP_MECHLAB_H
