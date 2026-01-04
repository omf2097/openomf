/*! @file
 * @brief Tournament selector widget for browsing available tournaments.
 */

#ifndef TRNSELECT_H
#define TRNSELECT_H

#include "formats/tournament.h"
#include "game/gui/component.h"

/** @brief Create a tournament selector widget. */
component *trnselect_create(void);

/** @brief Get the number of pilots in a PIC file. */
int trnselect_get_pilot_count(component *c, int pic_id);

/** @brief Select the next tournament. */
void trnselect_next(component *c);

/** @brief Select the previous tournament. */
void trnselect_prev(component *c);

/** @brief Get the currently selected tournament file. */
sd_tournament_file *trnselect_selected(component *c);

#endif // TRNSELECT_H
