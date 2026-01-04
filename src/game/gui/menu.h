/*! @file
 * @brief Menu container with keyboard navigation and submenu support.
 */

#ifndef MENU_H
#define MENU_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "video/surface.h"

/** @brief Per-frame tick callback type. */
typedef void (*menu_tick_cb)(component *c);

/** @brief Cleanup callback type. */
typedef void (*menu_free_cb)(component *c);

/** @brief Submenu completion callback type. */
typedef void (*menu_submenu_done_cb)(component *menu, component *submenu);

/** @brief Internal menu data. */
typedef struct menu {
    surface *bg1;      ///< Transparent background surface.
    surface *bg2;      ///< Solid background surface.
    surface *help_bg1; ///< Help text transparent background.
    surface *help_bg2; ///< Help text solid background.
    int selected;      ///< Index of currently selected child.
    int margin_top;    ///< Top margin in pixels.
    int padding;       ///< Padding between items in pixels.
    bool finished;     ///< True when menu exit is requested.
    bool horizontal;   ///< True for horizontal layout, false for vertical.
    bool background;   ///< Whether to render background surfaces.
    bool centered;     ///< Whether to center items within available space.
    bool is_submenu;   ///< Whether this menu is a submenu of another.

    int help_x;                             ///< Help text X position.
    int help_y;                             ///< Help text Y position.
    int help_w;                             ///< Help text area width.
    int help_h;                             ///< Help text area height.
    vga_index help_text_color;              ///< Help text color.
    text_horizontal_align help_text_halign; ///< Help text horizontal alignment.
    text_vertical_align help_text_valign;   ///< Help text vertical alignment.
    font_size help_text_font;               ///< Help text font.

    char prev_submenu_state;           ///< Previous submenu finished state (for detection).
    component *submenu;                ///< Active submenu component, or NULL.
    menu_submenu_done_cb submenu_done; ///< Callback when submenu finishes.

    void *userdata;    ///< User data pointer.
    menu_free_cb free; ///< Cleanup callback.
    menu_tick_cb tick; ///< Per-frame tick callback.
} menu;

/** @brief Create a new menu. */
component *menu_create(void);

/** @brief Add a child component to the menu. */
void menu_attach(component *menu, component *c);

/** @brief Select a specific child component. */
void menu_select(component *menu, component *c);

/** @brief Get the currently selected child. */
component *menu_selected(const component *menu);

/** @brief Check if the menu has finished (user exited). */
int menu_is_finished(const component *menu);

/** @brief Set a submenu to display. */
void menu_set_submenu(component *menu, component *submenu);

/** @brief Link to another menu frame. */
void menu_link_menu(component *menu, gui_frame *linked_menu);

/** @brief Get the current submenu. */
component *menu_get_submenu(const component *menu);

/** @brief Set callback for when submenu finishes. */
void menu_set_submenu_done_cb(component *menu, menu_submenu_done_cb done_cb);

/** @brief Set user data. */
void menu_set_userdata(component *menu, void *userdata);

/** @brief Get user data. */
void *menu_get_userdata(const component *menu);

/** @brief Set free callback. */
void menu_set_free_cb(component *menu, menu_free_cb cb);

/** @brief Set tick callback. */
void menu_set_tick_cb(component *menu, menu_tick_cb cb);

/** @brief Set horizontal or vertical layout. */
void menu_set_horizontal(component *c, bool horizontal);

/** @brief Enable or disable background rendering. */
void menu_set_background(component *c, bool background);

/** @brief Set help text position and size. */
void menu_set_help_pos(component *c, int x, int y, int h, int w);

/** @brief Configure help text appearance. */
void menu_set_help_text_settings(component *c, font_size font, text_horizontal_align halign, vga_index help_text_color);

/** @brief Set top margin. */
void menu_set_margin_top(component *c, int margin);

/** @brief Set padding between items. */
void menu_set_padding(component *c, int padding);

/** @brief Enable or disable item centering. */
void menu_set_centered(component *c, bool centered);

#endif // MENU_H
