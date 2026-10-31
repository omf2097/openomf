/**
 * @file colcoord.h
 * @brief Collision coordinate struct.
 * @details Struct for holding collision coordinate data for OMF:2097 animation structures.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef COLCOORD_H
#define COLCOORD_H

#include "utils/vec.h"
#include <stdint.h>

/** @brief Collision coordinate container
 *
 * Contains a single collision coordinate for animation frames. Collision coordinate
 * is a single point in 2D space, which is used to check for collisions between it
 * and another sprite. A single frame may contain multiple collision coordinates.
 * frame_id entry tells which sprite in animation the coordinate belongs to.
 */
typedef struct {
    vec2i pos;        ///< Position coordinate
    uint8_t null;     ///< Probably null padding
    uint8_t frame_id; ///< Sprite the coordinate belongs to
} sd_coord;

#endif // COLCOORD_H
