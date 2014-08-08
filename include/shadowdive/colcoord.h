/*! \file 
 * \brief Collision coordinate functions and types
 * \license MIT
 */ 

#ifndef _COL_COORD_H
#define _COL_COORD_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

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