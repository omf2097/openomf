/**
 * @file sprite_packer.h
 * @brief Rectangle packing algorithm for texture atlases.
 * @details Implements a bin-packing algorithm that allocates rectangular
 *          regions within a fixed-size area.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SPRITE_PACKER_H
#define SPRITE_PACKER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A rectangular region within the packing area.
 */
typedef struct sprite_region {
    uint16_t x; ///< X coordinate of the top-left corner
    uint16_t y; ///< Y coordinate of the top-left corner
    uint16_t w; ///< Width of the region
    uint16_t h; ///< Height of the region
} sprite_region;

/**
 * @brief Opaque rectangle packer structure.
 */
typedef struct sprite_packer sprite_packer;

/**
 * @brief Create a new rectangle packer.
 * @param width Width of the packing area
 * @param height Height of the packing area
 * @return Pointer to the new packer, or NULL on failure
 */
sprite_packer *sprite_packer_create(uint16_t width, uint16_t height);

/**
 * @brief Free a rectangle packer.
 * @param packer Pointer to the packer pointer (will be set to NULL)
 */
void sprite_packer_free(sprite_packer **packer);

/**
 * @brief Allocate a rectangular region.
 * @details Finds space for a rectangle of the given dimensions and returns
 *          its position. Uses a best-fit algorithm that prioritizes smaller
 *          regions to reduce fragmentation.
 * @param packer The packer to allocate from
 * @param w Width of the rectangle to allocate
 * @param h Height of the rectangle to allocate
 * @param out Output region with position and dimensions
 * @return true if allocation succeeded, false if no space available
 */
bool sprite_packer_alloc(sprite_packer *packer, uint16_t w, uint16_t h, sprite_region *out);

/**
 * @brief Reset the packer to its initial state.
 * @details Clears all allocations, making the entire area available again.
 * @param packer The packer to reset
 */
void sprite_packer_reset(sprite_packer *packer);

/**
 * @brief Get the width of the total area.
 * @param packer The packer to query
 * @return Width in pixels
 */
uint16_t sprite_packer_get_width(const sprite_packer *packer);

/**
 * @brief Get the height of the total area.
 * @param packer The packer to query
 * @return Height in pixels
 */
uint16_t sprite_packer_get_height(const sprite_packer *packer);

#endif // SPRITE_PACKER_H
