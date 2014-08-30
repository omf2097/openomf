/*! \file
 * \brief Collision coordinate struct.
 * \details Struct for holding collision coordinate data for OMF:2097 animation structures.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */ 

#ifndef _COL_COORD_H
#define _COL_COORD_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

/*! \brief Collision coordinate container
 *
 * Contains a single collision coordinate for animation frames. Collision coordinate
 * is a single point in 2D space, which is used to check for collisions between it
 * and another sprite. A single frame may contain multiple collision coordinates.
 * frame_id entry tells which sprite in animation the coordinate belongs to.
 */
typedef struct {
    int16_t x; ///< X position coordinate
    int16_t y; ///< Y position coordinate
    uint8_t null; ///< Probably null padding
    uint8_t frame_id; ///< Sprite the coordinate belongs to
} sd_coord;

#ifdef __cplusplus
}
#endif

#endif // _COL_COORD_H