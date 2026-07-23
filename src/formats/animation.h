/**
 * @file animation.h
 * @brief Animation handling.
 * @details Functions and structs for reading, writing and modifying OMF:2097 animation structures.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef SD_ANIMATION_H
#define SD_ANIMATION_H

#include "formats/colcoord.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/sprite.h"
#include "utils/str.h"
#include "utils/vec.h"
#include "utils/vector.h"
#include <stdint.h>

#define SD_ANIMATION_STRING_MAX 1024 ///< Maximum animation string size
#define SD_EXTRA_STRING_MAX 512      ///< Maximum extra string size

#define SD_SPRITE_COUNT_MAX 255   ///< Maximum amount of sprites allowed (technical limitation)
#define SD_COLCOORD_COUNT_MAX 256 ///< Maximum amount of collision coordinates allowed @todo find out the real maximum
#define SD_EXTRASTR_COUNT_MAX 10  ///< Maximum amount of extra strings.

/** @brief Generic animation container
 *
 * When starting animation playback, it should be positioned to the location pointed to by the
 * start position. Note that sprites may also have their own position offsets.
 *
 * Animation always has some amount of sprites and an animation string.
 * The string is used as a scripting language to tell the animation on how to go about it's business.
 * Animation string may change the animation position, playback speed, etc. etc.
 *
 * If the animation can collide with other objects, it may have an amount of collision coordinates.
 * A single sprite may have multiple collision coordinates assigned to it.
 */
typedef struct {
    vec2i start_pos;      ///< Animation start position
    int32_t null;         ///< Probably filler data
    vector coord_table;   ///< Collision coordinates, holds sd_coord elements
    vector extra_strings; ///< Extra animation strings, holds str elements
    vector sprites;       ///< Sprites, holds sd_sprite elements
    str anim_string;      ///< Animation string
} sd_animation;

/** @brief Initialize animation structure
 *
 * Initializes the animation structure with empty values.
 *
 * @param animation Allocated animation struct pointer.
 */
void sd_animation_create(sd_animation *animation);

/** @brief Copy animation structure
 *
 * Copies the contents of an animation structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_animation_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * animation structure, or problems are likely to appear.
 *
 * @param dst Destination animation struct pointer.
 * @param src Source animation struct pointer.
 */
void sd_animation_copy(sd_animation *dst, const sd_animation *src);

/** @brief Free animation structure
 *
 * Frees up all memory reserved by the animation structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * @param animation Animation struct to modify.
 */
void sd_animation_free(sd_animation *animation);

/** @brief Get sprite count
 *
 * Returns the sprite count in the animation.
 *
 * @param animation Animation struct to modify.
 * @return Sprite count
 */
int sd_animation_get_sprite_count(const sd_animation *animation);

/** @brief Sets sprite at index
 *
 * Sets the sprite at given index. It is only possible to overwrite values that
 * are already in the animation. Adding new values should be done by using
 * sd_animation_push_sprite().
 *
 * Any old data at given index will be automatically freed.
 *
 * @retval SD_INVALID_INPUT Invalid sprite index.
 * @retval SD_SUCCESS Success.
 *
 * @param animation Animation struct to modify
 * @param num Sprite index
 * @param sprite Sprite data. This will be copied.
 */
int sd_animation_set_sprite(sd_animation *animation, int num, const sd_sprite *sprite);

/** @brief Pushes a sprite to the end of the sprite list.
 *
 * Pushes a sprite to the end of the sprite list.
 * Sprite_count variable will be increased by 1.
 *
 * @retval SD_INVALID_INPUT Sprite list is already full
 * @retval SD_SUCCESS Success.
 *
 * @param animation Animation struct to modify
 * @param sprite Sprite information. This will be copied.
 */
int sd_animation_push_sprite(sd_animation *animation, const sd_sprite *sprite);

/** @brief Pops a sprite off from the end of the sprite list.
 *
 * Pops a sprite off the end of the sprite list.
 * Sprite_count variable will be decreased by 1.
 * Popped data will be automatically freed.
 *
 * @retval SD_INVALID_INPUT Sprite list is already empty
 * @param animation Animation struct to modify
 */
void sd_animation_pop_sprite(sd_animation *animation);

/** @brief Get sprite at given index
 *
 * Returns a pointer to the sprite data at given index.
 *
 * @retval NULL There was no sprite at given index
 * @retval sd_sprite* Success.
 *
 * @param animation Animation struct to modify.
 * @param num Sprite index
 */
sd_sprite *sd_animation_get_sprite(sd_animation *animation, int num);

/** @brief Load animation from an open reader
 *
 * @retval SD_FILE_PARSE_ERROR File does not contain valid data.
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open reader to read from.
 * @param animation Animation struct to fill.
 */
int sd_animation_load(sd_reader *reader, sd_animation *animation);

/** @brief Save animation to an open writer
 *
 * @retval SD_SUCCESS Success.
 *
 * @param writer Open writer to write to.
 * @param animation Animation struct to save.
 */
int sd_animation_save(sd_writer *writer, const sd_animation *animation);

#endif // SD_ANIMATION_H
