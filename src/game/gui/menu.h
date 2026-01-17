/**
 * @file menu.h
 * @brief GUI menu widget
 * @details A menu container for organizing selectable items with optional submenus.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef MENU_H
#define MENU_H

#include "game/gui/component.h"
#include "game/gui/gui_frame.h"
#include "video/surface.h"

typedef void (*menu_tick_cb)(component *c);                                ///< Menu tick callback
typedef void (*menu_free_cb)(component *c);                                ///< Menu free callback
typedef void (*menu_submenu_done_cb)(component *menu, component *submenu); ///< Submenu completion callback

/**
 * @brief Menu internal structure
 */
typedef struct menu {
    surface *bg1;      ///< Primary background surface
    surface *bg2;      ///< Secondary background surface
    surface *help_bg1; ///< Primary help area background
    surface *help_bg2; ///< Secondary help area background
    int selected;      ///< Index of selected item
    int margin_top;    ///< Top margin in pixels
    int padding;       ///< Padding between items
    bool finished;     ///< Whether the menu is finished
    bool horizontal;   ///< Whether items are arranged horizontally
    bool background;   ///< Whether to draw background
    bool centered;     ///< Whether items are centered
    bool is_submenu;   ///< Whether this menu is a submenu

    int help_x;                             ///< Help area X coordinate
    int help_y;                             ///< Help area Y coordinate
    int help_w;                             ///< Help area width
    int help_h;                             ///< Help area height
    vga_index help_text_color;              ///< Help text color
    text_horizontal_align help_text_halign; ///< Help text horizontal alignment
    text_vertical_align help_text_valign;   ///< Help text vertical alignment
    font_size help_text_font;               ///< Help text font

    char prev_submenu_state;           ///< Previous submenu state
    component *submenu;                ///< Active submenu
    menu_submenu_done_cb submenu_done; ///< Submenu completion callback

    void *userdata;    ///< User data for callbacks
    menu_free_cb free; ///< Free callback
    menu_tick_cb tick; ///< Tick callback
} menu;

/**
 * @brief Create a menu
 * @return Pointer to the newly created menu component
 */
component *menu_create(void);

/**
 * @brief Attach a component to the menu
 * @param menu Menu to attach to
 * @param c Component to attach
 */
void menu_attach(component *menu, component *c);

/**
 * @brief Select a specific component in the menu
 * @param menu Menu to modify
 * @param c Component to select
 */
void menu_select(component *menu, component *c);

/**
 * @brief Get the currently selected component
 * @param menu Menu to query
 * @return Pointer to the selected component
 */
component *menu_selected(const component *menu);

/**
 * @brief Check if the menu is finished
 * @param menu Menu to query
 * @return Non-zero if finished
 */
int menu_is_finished(const component *menu);

/**
 * @brief Set the active submenu
 * @param menu Menu to modify
 * @param submenu Submenu component to set
 */
void menu_set_submenu(component *menu, component *submenu);

/**
 * @brief Link this menu to a GUI frame
 * @param menu Menu to modify
 * @param linked_menu GUI frame to link
 */
void menu_link_menu(component *menu, gui_frame *linked_menu);

/**
 * @brief Get the current submenu
 * @param menu Menu to query
 * @return Pointer to the current submenu, or NULL if none
 */
component *menu_get_submenu(const component *menu);

/**
 * @brief Set the submenu completion callback
 * @param menu Menu to modify
 * @param done_cb Callback invoked when submenu is done
 */
void menu_set_submenu_done_cb(component *menu, menu_submenu_done_cb done_cb);

/**
 * @brief Set user data for the menu
 * @param menu Menu to modify
 * @param userdata User data pointer to store
 */
void menu_set_userdata(component *menu, void *userdata);

/**
 * @brief Get user data from the menu
 * @param menu Menu to query
 * @return Stored user data pointer
 */
void *menu_get_userdata(const component *menu);

/**
 * @brief Set the free callback
 * @param menu Menu to modify
 * @param cb Free callback function
 */
void menu_set_free_cb(component *menu, menu_free_cb cb);

/**
 * @brief Set the tick callback
 * @param menu Menu to modify
 * @param cb Tick callback function
 */
void menu_set_tick_cb(component *menu, menu_tick_cb cb);

/**
 * @brief Set whether items are arranged horizontally
 * @param c Menu component to modify
 * @param horizontal True for horizontal layout, false for vertical
 */
void menu_set_horizontal(component *c, bool horizontal);

/**
 * @brief Set whether to draw the menu background
 * @param c Menu component to modify
 * @param background True to draw background
 */
void menu_set_background(component *c, bool background);

/**
 * @brief Set the help area position and size
 * @param c Menu component to modify
 * @param x X coordinate of help area
 * @param y Y coordinate of help area
 * @param h Height of help area
 * @param w Width of help area
 */
void menu_set_help_pos(component *c, int x, int y, int h, int w);

/**
 * @brief Set the help text display settings
 * @param c Menu component to modify
 * @param font Font size for help text
 * @param halign Horizontal alignment for help text
 * @param help_text_color Color for help text
 */
void menu_set_help_text_settings(component *c, font_size font, text_horizontal_align halign, vga_index help_text_color);

/**
 * @brief Set the top margin
 * @param c Menu component to modify
 * @param margin Margin in pixels
 */
void menu_set_margin_top(component *c, int margin);

/**
 * @brief Set the padding between items
 * @param c Menu component to modify
 * @param padding Padding in pixels
 */
void menu_set_padding(component *c, int padding);

/**
 * @brief Set whether items are centered
 * @param c Menu component to modify
 * @param centered True to center items
 */
void menu_set_centered(component *c, bool centered);

#endif // MENU_H
