/*! @file
 * @brief Visual gauge widget displaying lit/unlit segments.
 */

#ifndef GAUGE_H
#define GAUGE_H

#include "game/gui/component.h"

/** @brief Gauge visual styles. */
typedef enum gauge_type
{
    GAUGE_SMALL, ///< 3x3 pixel segments.
    GAUGE_BIG    ///< 8x3 pixel segments.
} gauge_type;

/** @brief Create a gauge widget. */
component *gauge_create(gauge_type type, int size, int lit);

/** @brief Set the number of lit segments. */
void gauge_set_lit(component *gauge, int lit);

/** @brief Set the total number of segments. */
void gauge_set_size(component *gauge, int size);

/** @brief Get the number of lit segments. */
int gauge_get_lit(component *gauge);

/** @brief Get the total number of segments. */
int gauge_get_size(component *gauge);

#endif // GAUGE_H
