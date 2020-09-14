/*! \file
 * \brief Animation handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 animation structures.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#include <stdint.h>
#include "formats/sprite.h"
#include "formats/colcoord.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "utils/str.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SD_SPRITE_COUNT_MAX 255 ///< Maximum amount of sprites allowed (technical limitation)
#define SD_COLCOORD_COUNT_MAX 256 ///< Maximum amount of collision coordinates allowed \todo find out the real maximum
#define SD_EXTRASTR_COUNT_MAX 10 ///< Maximum amount of extra strings.

/*! \brief Generic animation container
 *
 * When starting animation playback, it should be positioned to the location pointed to by the
 * start_x, start_y variables. Note that sprites may also have their own position offsets.
 *
 * Animation always has some amount of sprites and an animation string.
 * The string is used as a scripting language to tell the animation on how to go about it's business.
 * Animation string may change the animation position, playback speed, etc. etc.
 *
 * If the animation can collide with other objects, it may have an amount of collision coordinates.
 * A single sprite may have multiple collision coordinates assigned to it.
 */
typedef struct {
    // Header
    int16_t start_x; ///< Animation start position, X-axis
    int16_t start_y; ///< Animation start position, Y-axis
    int32_t null; ///< Probably filler data
    uint16_t coord_count; ///< Number of collision coordinates in animation frames
    uint8_t sprite_count; ///< Number of sprites in animation
    uint8_t extra_string_count; ///< Number of extra strings in animation

    // Sprites and their collision coordinates
    sd_coord coord_table[SD_COLCOORD_COUNT_MAX]; ///< Collision coordinates
    sd_sprite *sprites[SD_SPRITE_COUNT_MAX]; ///< Sprites

    // String header & Extra strings
    str anim_string; ///< Animation string
    str extra_strings[SD_EXTRASTR_COUNT_MAX]; ///< Extra strings
} sd_animation;

/*! \brief Initialize animation structure
 *
 * Initializes the animation structure with empty values.
 *
 * \retval SD_INVALID_INPUT Animation struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param animation Allocated animation struct pointer.
 */
int sd_animation_create(sd_animation* animation);

/*! \brief Copy animation structure
 *
 * Copies the contents of an animation structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_animation_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * animation structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination animation struct pointer.
 * \param src Source animation struct pointer.
 */
int sd_animation_copy(sd_animation *dst, const sd_animation *src);

/*! \brief Free animation structure
 *
 * Frees up all memory reserved by the animation structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param animation Animation struct to modify.
 */
void sd_animation_free(sd_animation *animation);

/*! \brief Get coordinate count
 *
 * Returns the collision coordinate count in the animation.
 *
 * \param animation Animation struct to modify.
 * \return Coordinate element count
 */
int sd_animation_get_coord_count(sd_animation *animation);

/*! \brief Sets coordinate at index
 *
 * Sets the coordinate at given index. It is only possible to overwrite values that
 * are already in the animation. Adding new values should be done by using sd_animation_push_coord().
 *
 * \retval SD_INVALID_INPUT Invalid coordinate index
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param num Coordinate index
 * \param coord Coordinate information.
 */
int sd_animation_set_coord(sd_animation *animation, int num, const sd_coord coord);

/*! \brief Pushes coordinate to the end of coordinate list.
 *
 * Pushes a coordinate to the end of the coordinate list.
 * Coord_count variable will be raised by 1.
 *
 * \retval SD_INVALID_INPUT Coordinate list is already full
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param coord Coordinate information.
 */
int sd_animation_push_coord(sd_animation *animation, const sd_coord coord);

/*! \brief Pops a coordinate from the end of the coordinate list.
 *
 * Pops a coordinate off the end of the coordinate list.
 * Coord_count variable will be decreased by 1.
 *
 * \retval SD_INVALID_INPUT Coordinate list is already empty
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 */
int sd_animation_pop_coord(sd_animation *animation);

/*! \brief Gets a coordinate pointer at index
 *
 * Returns a pointer to the coordinate data at given index.
 *
 * \retval NULL There is no coordinate at the given index
 * \retval sd_coord* Success
 *
 * \param animation Animation struct to modify
 * \param num Coordinate index
 */
sd_coord* sd_animation_get_coord(sd_animation *animation, int num);

/*! \brief Sets the animation string
 *
 * Sets the animation string for the given animation. String will be copied.
 *
 * \retval SD_INVALID_INPUT Given string was too big.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param str New animation string
 */
int sd_animation_set_anim_string(sd_animation *animation, const str *src);

/*! \brief Get extra string count
 *
 * Returns the extra string count in the animation.
 *
 * \param animation Animation struct to modify.
 * \return Extra string count
 */
int sd_animation_get_extra_string_count(sd_animation *animation);

/*! \brief Sets extra string at index
 *
 * Sets the extra string at given index. It is only possible to overwrite values that
 * are already in the animation. Adding new values should be done by using
 * sd_animation_push_extra_string().
 *
 * Maximum extra string length is 512 bytes.
 *
 * \retval SD_INVALID_INPUT Invalid extra string index or string too long.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param num String index
 * \param str Extra string. This will be copied.
 */
int sd_animation_set_extra_string(sd_animation *animation, int num, const str *src);

/*! \brief Pushes extra string to the end of string list.
 *
 * Pushes am extra string to the end of the extra string list.
 * Extra_string_count variable will be increased by 1.
 *
 * \retval SD_INVALID_INPUT Extra string list is full or string too long.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param str Extra string. This will be copied.
 */
int sd_animation_push_extra_string(sd_animation *animation, const str *src);

/*! \brief Pops an extra string off from the end of the extra string list.
 *
 * Pops a extra string off the end of the extra string list.
 * Extra_string_count variable will be decreased by 1.
 *
 * \retval SD_INVALID_INPUT Extra string list is already empty.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 */
int sd_animation_pop_extra_string(sd_animation *animation);

/*! \brief Get extra string at given index
 *
 * Returns the extra string at given index.
 *
 * \retval NULL There is no extra string at given index.
 * \retval str* Pointer to the extra string at given index.
 *
 * \param animation Animation struct to modify.
 * \param num Extra string index
 */
str* sd_animation_get_extra_string(sd_animation *animation, int num);

/*! \brief Get extra string count
 *
 * Returns the extra string count in the animation.
 *
 * \param animation Animation struct to modify.
 * \return Extra string count
 */
int sd_animation_get_sprite_count(sd_animation *animation);

/*! \brief Sets sprite at index
 *
 * Sets the sprite at given index. It is only possible to overwrite values that
 * are already in the animation. Adding new values should be done by using
 * sd_animation_push_sprite().
 *
 * Any old data at given index will be automatically freed.
 *
 * \retval SD_INVALID_INPUT Invalid sprite index or sprite was NULL.
 * \retval SD_OUT_OF_MEMORY Memory ran out. Data at the given index will be NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param num Sprite index
 * \param sprite Sprite data. This will be copied.
 */
int sd_animation_set_sprite(sd_animation *animation, int num, const sd_sprite *sprite);

/*! \brief Pushes a sprite to the end of the sprite list.
 *
 * Pushes a sprite to the end of the sprite list.
 * Sprite_count variable will be increased by 1.
 *
 * \retval SD_INVALID_INPUT Coordinate list is already full
 * \retval SD_OUT_OF_MEMORY Memory ran out. Animation will not be affected.
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 * \param sprite Sprite information. This will be copied.
 */
int sd_animation_push_sprite(sd_animation *animation, const sd_sprite *sprite);

/*! \brief Pops a sprite off from the end of the sprite list.
 *
 * Pops a sprite off the end of the sprite list.
 * Sprite_count variable will be decreased by 1.
 * Popped data will be automatically freed.
 *
 * \retval SD_INVALID_INPUT Sprite list is already empty
 * \retval SD_SUCCESS Success.
 *
 * \param animation Animation struct to modify
 */
int sd_animation_pop_sprite(sd_animation *animation);

/*! \brief Get sprite at given index
 *
 * Returns a pointer to the sprite data at given index.
 *
 * \retval NULL There was no sprite at given index
 * \retval sd_sprite* Success.
 *
 * \param animation Animation struct to modify.
 * \param num Sprite index
 */
sd_sprite* sd_animation_get_sprite(sd_animation *animation, int num);


int sd_animation_load(sd_reader *reader, sd_animation *animation);
int sd_animation_save(sd_writer *writer, const sd_animation *animation);


#ifdef __cplusplus
}
#endif

#endif // _SD_ANIMATION_H
