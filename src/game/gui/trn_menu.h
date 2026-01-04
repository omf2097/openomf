/*! @file
 * @brief Tournament-style menu with animated button sprites and submenu support.
 */

#ifndef TRN_MENU_H
#define TRN_MENU_H

#include "game/game_state.h"
#include "game/gui/component.h"
#include "game/protos/object.h"
#include "resources/animation.h"
#include "utils/vec.h"

/** @brief Per-frame tick callback. */
typedef void (*trnmenu_tick_cb)(component *c);

/** @brief Cleanup callback when menu is freed. */
typedef void (*trnmenu_free_cb)(component *c);

/** @brief Callback when submenu is initialized. */
typedef void (*trnmenu_submenu_init_cb)(component *menu, component *submenu);

/** @brief Callback when submenu is closed. */
typedef void (*trnmenu_submenu_done_cb)(component *menu, component *submenu);

/** @brief Create a tournament menu. */
component *trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y, bool return_hand);

/** @brief Attach a child component to the menu. */
void trnmenu_attach(component *menu, component *c);

/** @brief Bind the hand cursor animation. */
void trnmenu_bind_hand(component *menu, animation *hand, game_state *gs);

/** @brief Set the active submenu. */
void trnmenu_set_submenu(component *menu, component *submenu);

/** @brief Get the active submenu. */
component *trnmenu_get_submenu(const component *menu);

/** @brief Set the submenu initialization callback. */
void trnmenu_set_submenu_init_cb(component *menu, trnmenu_submenu_init_cb done_cb);

/** @brief Set the submenu close callback. */
void trnmenu_set_submenu_done_cb(component *menu, trnmenu_submenu_done_cb done_cb);

/** @brief Check if menu exit animation is finished. */
int trnmenu_is_finished(const component *menu);

/** @brief Begin menu exit animation. */
void trnmenu_finish(component *menu);

/** @brief Check if menu is in fade transition. */
int trnmenu_is_fading(const component *menu);

/** @brief Set user data associated with this menu. */
void trnmenu_set_userdata(component *menu, void *userdata);

/** @brief Get user data associated with this menu. */
void *trnmenu_get_userdata(const component *menu);

/** @brief Set the cleanup callback. */
void trnmenu_set_free_cb(component *menu, trnmenu_free_cb cb);

/** @brief Set the per-frame tick callback. */
void trnmenu_set_tick_cb(component *menu, trnmenu_tick_cb cb);

#endif // TRN_MENU_H
