/**
 * @file portrait.h
 * @brief GUI portrait widget
 * @details Widget for displaying pilot portraits from PIC files.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef PORTRAIT_H
#define PORTRAIT_H

#include "formats/sprite.h"
#include "game/gui/component.h"

/**
 * @brief Create a portrait widget
 * @param pilot_id Pilot index within the PIC file
 * @return Pointer to the newly created portrait component
 */
component *portrait_create(int pilot_id);

/**
 * @brief Select a portrait by PIC and pilot ID
 * @param c Portrait component to modify
 * @param pilot_id Pilot index within the PIC file
 */
void portrait_select(component *c, int pilot_id);

/**
 * @brief Select a portrait by PIC and pilot ID with extended palette slot
 * @param c Portrait component to modify
 * @param pilot_id Pilot index within the PIC file
 * @param slot_index Portrait slot (0-4) for extended palette remapping
 */
void portrait_select_with_slot(component *c, int pilot_id, int slot_index);

/**
 * @brief Get the number of pilots in a PIC file
 * @param c Portrait component to query
 * @return Number of pilots available
 */
int portrait_get_pilot_count(component *c);

/**
 * @brief Load a portrait sprite and palette
 * @param s Output sprite structure
 * @param pal Output palette structure
 * @param pilot_id Pilot index within the PIC file
 * @return Zero on success, non-zero on failure
 */
int portrait_load(sd_sprite *s, vga_palette *pal, int pilot_id);

/**
 * @brief Load a portrait sprite and palette with extended palette slot assignment
 * @param s Output sprite structure
 * @param pal Output palette structure
 * @param pilot_id Pilot index within the PIC file
 * @param slot_index Portrait slot (0-4) for extended palette remapping
 * @return Zero on success, non-zero on failure
 */
int portrait_load_with_slot(sd_sprite *s, vga_palette *pal, int pilot_id, int slot_index);

/**
 * @brief Select the next portrait
 * @param c Portrait component to modify
 */
void portrait_next(component *c);

/**
 * @brief Select the previous portrait
 * @param c Portrait component to modify
 */
void portrait_prev(component *c);

/**
 * @brief Get the currently selected pilot ID
 * @param c Portrait component to query
 * @return Currently selected pilot ID
 */
int portrait_selected(component *c);

/**
 * @brief Set portrait from an existing sprite
 * @param c Portrait component to modify
 * @param spr Sprite to display
 */
void portrait_set_from_sprite(component *c, sd_sprite *spr, int slot_index);

#endif // PORTRAIT_H
