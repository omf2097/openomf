/**
 * @file gauge.h
 * @brief GUI gauge widget
 * @details A gauge widget displaying a series of lit/unlit segments (like power bars).
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef GAUGE_H
#define GAUGE_H

#include "game/gui/component.h"

/**
 * @brief Gauge size types
 */
typedef enum gauge_type
{
    GAUGE_SMALL, ///< Small gauge segments
    GAUGE_BIG    ///< Large gauge segments
} gauge_type;

/**
 * @brief Create a gauge widget
 * @param type Size type of the gauge
 * @param size Total number of segments
 * @param lit Number of lit (active) segments
 * @return Pointer to the newly created gauge component
 */
component *gauge_create(gauge_type type, int size, int lit);

/**
 * @brief Set the number of lit segments
 * @param gauge Gauge component to modify
 * @param lit Number of lit segments
 */
void gauge_set_lit(component *gauge, int lit);

/**
 * @brief Set the total number of segments
 * @param gauge Gauge component to modify
 * @param size Total number of segments
 */
void gauge_set_size(component *gauge, int size);

/**
 * @brief Get the number of lit segments
 * @param gauge Gauge component to query
 * @return Number of lit segments
 */
int gauge_get_lit(component *gauge);

/**
 * @brief Get the total number of segments
 * @param gauge Gauge component to query
 * @return Total number of segments
 */
int gauge_get_size(component *gauge);

#endif // GAUGE_H
