/**
 * @file trnselect.h
 * @brief Tournament selection widget
 * @details Widget for selecting tournament files.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TRNSELECT_H
#define TRNSELECT_H

#include "formats/tournament.h"
#include "game/gui/component.h"

/**
 * @brief Create a tournament selection widget
 * @return Pointer to the newly created tournament selector component
 */
component *trnselect_create(void);

/**
 * @brief Get the number of pilots for a given PIC ID
 * @param c Tournament selector component to query
 * @param pic_id PIC file ID
 * @return Number of pilots available
 */
int trnselect_get_pilot_count(component *c, int pic_id);

/**
 * @brief Select the next tournament
 * @param c Tournament selector component to modify
 */
void trnselect_next(component *c);

/**
 * @brief Select the previous tournament
 * @param c Tournament selector component to modify
 */
void trnselect_prev(component *c);

/**
 * @brief Get the currently selected tournament file
 * @param c Tournament selector component to query
 * @return Pointer to the selected tournament file structure
 */
sd_tournament_file *trnselect_selected(component *c);

#endif // TRNSELECT_H
