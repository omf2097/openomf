/*! @file
 * @brief Portrait widget for displaying fighter/pilot portraits.
 */

#ifndef PORTRAIT_H
#define PORTRAIT_H

#include "formats/sprite.h"
#include "game/gui/component.h"

/** @brief Create a portrait widget. */
component *portrait_create(int pic_id, int pilot_id);

/** @brief Select a specific portrait. */
void portrait_select(component *c, int pic_id, int pilot_id);

/** @brief Get the number of pilots in a PIC file. */
int portrait_get_pilot_count(component *c, int pic_id);

/** @brief Load a portrait sprite and palette from resources. */
int portrait_load(sd_sprite *s, vga_palette *pal, int pic_id, int pilot_id);

/** @brief Select the next portrait (wraps to first). */
void portrait_next(component *c);

/** @brief Select the previous portrait (wraps to last). */
void portrait_prev(component *c);

/** @brief Get the currently selected pilot index. */
int portrait_selected(component *c);

/** @brief Set portrait from raw sprite data. */
void portrait_set_from_sprite(component *c, sd_sprite *spr);

#endif // PORTRAIT_H
