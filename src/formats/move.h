/*! \file
 * \brief AF move handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 Fighter specific move structures.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_MOVE_H
#define SD_MOVE_H

#include "formats/animation.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include <stdint.h>

#define SD_MOVE_STRING_MAX 21         ///< Maximum allowed move string length
#define SD_MOVE_FOOTER_STRING_MAX 512 ///< Maximum allowed footer string length

/*! \brief HAR Move information
 *
 * Contains information about the HAR move. Wraps a generic animation.
 * \todo Find out what the unknown fields are.
 */
typedef struct {
    sd_animation *animation; ///< Animation field for Move. When saving AF file, this should be != NULL.

    uint16_t ai_opts;        ///< Unknown value
    uint16_t pos_constraint; ///< Unknown value
    uint8_t unknown_4;       ///< Unknown value
    uint8_t unknown_5;       ///< Unknown value
    uint8_t unknown_6;       ///< Unknown value
    uint8_t unknown_7;       ///< Unknown value
    uint8_t unknown_8;       ///< Unknown value
    uint8_t unknown_9;       ///< Unknown value
    uint8_t unknown_10;      ///< Unknown value
    uint8_t unknown_11;      ///< Unknown value
    uint8_t next_anim_id;    ///< Next animation ID
    uint8_t category;        ///< Move category ID
    uint8_t block_damage;    ///< Damage applied when blocking this mode
    uint8_t block_stun;   ///< How many frames to force the opponents block animation on hit, also used for scrap amount
    uint8_t successor_id; ///< Successor animation ID
    uint8_t damage_amount;         ///< Damage amount when this move connects
    uint8_t collision_opts;        ///< Unknown value
    uint8_t extra_string_selector; ///< what upgrades change the animation string
    uint8_t points;                ///< Score gained for this hit

    char move_string[SD_MOVE_STRING_MAX];          ///< Move string
    char footer_string[SD_MOVE_FOOTER_STRING_MAX]; ///< Footer string
} sd_move;

/*! \brief Initialize Move structure
 *
 * Initializes the move structure with empty values.
 *
 * \retval SD_INVALID_INPUT BK struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param move Allocated move struct pointer.
 */
int sd_move_create(sd_move *move);

/*! \brief Copy Move structure
 *
 * Copies the contents of an move structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_move_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * move structure, or problems are likely to appear.
 *
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination move struct pointer.
 * \param src Source move struct pointer.
 */
int sd_move_copy(sd_move *dst, const sd_move *src);

/*! \brief Free move structure
 *
 * Frees up all memory reserved by the move structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param move Move struct to modify.
 */
void sd_move_free(sd_move *move);

/*! \brief Set animation struct for move
 *
 * Sets an animation for the move. Animation will be copied,
 * so remember to free your local copy yourself. Note that any valid
 * move struct should ALWAYS contain an animation. Otherwise there will be problems
 * eg. saving the AF file.
 *
 * A NULL value for animation field will result in move->animation field getting freed.
 *
 * \retval SD_INVALID_INPUT Move struct pointer was NULL.
 * \retval SD_SUCCESS on success.
 *
 * \param move Move struct to modify.
 * \param animation Animation to set. This will be copied.
 */
int sd_move_set_animation(sd_move *move, const sd_animation *animation);

/*! \brief Get the current animation
 *
 * Returns a pointer to the current animation for the move. If animation
 * is not set, NULL will be returned.
 *
 * \retval NULL Animation does not exist
 * \retval sd_animation* Success
 *
 * \param move Move struct to modify.
 */
sd_animation *sd_move_get_animation(const sd_move *move);

/*! \brief Set move footer string for the Move struct.
 *
 * Sets the move footer string for the Move struct. Maximum length is
 * 512 bytes. Longer strings will result in error.
 *
 * \retval SD_INVALID_INPUT Input string was too long.
 * \retval SD_SUCCESS Success.
 *
 * \param move Move struct to modify.
 * \param str String to set.
 */
int sd_move_set_footer_string(sd_move *move, const char *str);

/*! \brief Set move string for the Move struct.
 *
 * Sets the move string for the Move struct. Maximum length is
 * 21 bytes. Longer strings will result in error.
 *
 * \retval SD_INVALID_INPUT Input string was too long.
 * \retval SD_SUCCESS Success.
 *
 * \param move Move struct to modify.
 * \param str String to set.
 */
int sd_move_set_move_string(sd_move *move, const char *str);

int sd_move_load(sd_reader *reader, sd_move *move);
int sd_move_save(sd_writer *writer, const sd_move *move);

#endif // SD_MOVE_H
