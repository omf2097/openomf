/**
 * @file trn_menu.h
 * @brief Tournament menu system
 * @details Menu system specifically for tournament mode with animated hand cursor.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TRN_MENU_H
#define TRN_MENU_H

#include "game/game_state.h"
#include "game/gui/component.h"
#include "game/protos/object.h"
#include "resources/animation.h"

typedef void (*trnmenu_tick_cb)(component *c);                                ///< Menu tick callback
typedef void (*trnmenu_free_cb)(component *c);                                ///< Menu free callback
typedef void (*trnmenu_submenu_init_cb)(component *menu, component *submenu); ///< Submenu initialization callback
typedef void (*trnmenu_submenu_done_cb)(component *menu, component *submenu); ///< Submenu completion callback

/**
 * @brief Create a tournament menu
 * @param button_sheet Sprite sheet containing button graphics
 * @param sheet_x X offset in the sprite sheet
 * @param sheet_y Y offset in the sprite sheet
 * @param return_hand Whether to animate the hand cursor returning
 * @return Pointer to the newly created tournament menu component
 */
component *trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y, bool return_hand);

/**
 * @brief Attach a component to the tournament menu
 * @param menu Tournament menu to attach to
 * @param c Component to attach
 */
void trnmenu_attach(component *menu, component *c);

/**
 * @brief Bind the animated hand cursor to the menu
 * @param menu Tournament menu to modify
 * @param hand Animation for the hand cursor
 * @param gs Game state for animation context
 */
void trnmenu_bind_hand(component *menu, animation *hand, game_state *gs);

/**
 * @brief Set the active submenu
 * @param menu Tournament menu to modify
 * @param submenu Submenu component to set
 */
void trnmenu_set_submenu(component *menu, component *submenu);

/**
 * @brief Get the current submenu
 * @param menu Tournament menu to query
 * @return Pointer to the current submenu, or NULL if none
 */
component *trnmenu_get_submenu(const component *menu);

/**
 * @brief Set the submenu initialization callback
 * @param menu Tournament menu to modify
 * @param done_cb Callback invoked when submenu is initialized
 */
void trnmenu_set_submenu_init_cb(component *menu, trnmenu_submenu_init_cb done_cb);

/**
 * @brief Set the submenu completion callback
 * @param menu Tournament menu to modify
 * @param done_cb Callback invoked when submenu is done
 */
void trnmenu_set_submenu_done_cb(component *menu, trnmenu_submenu_done_cb done_cb);

/**
 * @brief Check if the menu has finished
 * @param menu Tournament menu to query
 * @return Non-zero if menu is finished
 */
int trnmenu_is_finished(const component *menu);

/**
 * @brief Mark the menu as finished
 * @param menu Tournament menu to modify
 */
void trnmenu_finish(component *menu);

/**
 * @brief Check if the menu is currently fading
 * @param menu Tournament menu to query
 * @return Non-zero if menu is fading
 */
int trnmenu_is_fading(const component *menu);

/**
 * @brief Set user data for the menu
 * @param menu Tournament menu to modify
 * @param userdata User data pointer to store
 */
void trnmenu_set_userdata(component *menu, void *userdata);

/**
 * @brief Get user data from the menu
 * @param menu Tournament menu to query
 * @return Stored user data pointer
 */
void *trnmenu_get_userdata(const component *menu);

/**
 * @brief Set the free callback
 * @param menu Tournament menu to modify
 * @param cb Free callback function
 */
void trnmenu_set_free_cb(component *menu, trnmenu_free_cb cb);

/**
 * @brief Set the tick callback
 * @param menu Tournament menu to modify
 * @param cb Tick callback function
 */
void trnmenu_set_tick_cb(component *menu, trnmenu_tick_cb cb);

#endif // TRN_MENU_H
