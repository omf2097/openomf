#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    APCONN_NOT_CONNECTED = 0,
    APCONN_CONNECTING,
    APCONN_READY,
    APCONN_FATAL_ERROR,
} ap_connection_status_t;

#ifdef __cplusplus
extern "C" {
#endif

// Lifecycle
void Archipelago_Connect(const char *uri, const char *slot, const char *password);
void Archipelago_Poll(void);
void Archipelago_Disconnect(void);
ap_connection_status_t Archipelago_ConnectionStatus(void);

// Send a location check to the server.
void Archipelago_SendCheck(int64_t location_id);

// Signal goal completion.
void Archipelago_GoalComplete(void);

// Register a callback invoked when any item is received.
// item_name and player_name are transient — copy if needed beyond the call.
void Archipelago_SetItemReceivedCallback(void (*cb)(const char *item_name, const char *player_name));

// Scout (and hint) a buy location. Response arrives via the buy hint callback.
void Archipelago_ScoutBuyLocation(int64_t location_id);

// Register a callback invoked when scout info arrives for a buy location.
// location_id, item_name, and player_name are transient — copy if needed.
void Archipelago_SetBuyHintCallback(void (*cb)(int64_t location_id, const char *item_name, const char *player_name));

// Register a callback invoked once after each on_items_received batch completes.
// Fires after all per-item callbacks, so APItems and APChecks are fully rebuilt.
void Archipelago_SetItemsDoneCallback(void (*cb)(void));

// Write a deterministic save-file identifier derived from the current seed + slot
// into out (up to len bytes, including NUL). Format: "AP" + 8 uppercase hex digits.
// Must be called after the slot is connected.
void Archipelago_GetSaveIdent(char *out, size_t len);

// Write the connected slot name into out (up to len bytes, including NUL).
void Archipelago_GetSlotName(char *out, size_t len);

// Persist APSave (har_money, last_applied_item_index, tournaments_won_mask,
// money_small_received, money_large_received) to <ident>.APS.
void Archipelago_APSaveState(const char *ident);

// Load APSave from <ident>.APS. Returns true on success, false if the file doesn't exist.
bool Archipelago_APLoadState(const char *ident);

// Pure award formula for a money check: base + step * received_count. Exposed for unit tests.
int32_t ap_money_award(int32_t base, int32_t step, uint32_t received_count);

#ifdef __cplusplus
}
#endif
