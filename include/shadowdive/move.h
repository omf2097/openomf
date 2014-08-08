/*! \file 
 * \brief Contains functions for handling AF move data structures.
 * \license MIT
 */ 

#ifndef _SD_MOVE_H
#define _SD_MOVE_H

#include <stdint.h>
#include "shadowdive/animation.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_MOVE_STRING_MAX 21
#define SD_MOVE_FOOTER_STRING_MAX 512

typedef struct {
    sd_animation *animation; ///< Animation field for Move. When saving AF file, this should be != NULL.
    
    uint16_t unknown_0; ///< Unknown value
    uint16_t unknown_2; ///< Unknown value
    uint8_t unknown_4; ///< Unknown value
    uint8_t unknown_5; ///< Unknown value
    uint8_t unknown_6; ///< Unknown value
    uint8_t unknown_7; ///< Unknown value
    uint8_t unknown_8; ///< Unknown value
    uint8_t unknown_9; ///< Unknown value
    uint8_t unknown_10; ///< Unknown value
    uint8_t unknown_11; ///< Unknown value
    uint8_t next_anim_id; ///< Next animation ID
    uint8_t category; ///< Move category ID
    uint8_t unknown_14; ///< Unknown value
    uint8_t scrap_amount; ///< Scrap amount when this move connects
    uint8_t successor_id; ///< Successor animation ID
    uint8_t damage_amount; ///< Damage amount when this move connects
    uint8_t unknown_18; ///< Unknown value
    uint8_t unknown_19; ///< Unknown value
    uint8_t points;  ///< Score gained for this hit

    char move_string[SD_MOVE_STRING_MAX];  ///< Move string
    char footer_string[SD_MOVE_FOOTER_STRING_MAX]; ///< Footer string
} sd_move;

/*! \brief Initialize Move structure
 *
 * Initializes the move structure with empty values.
 *
 * Return values:
 * - SD_INVALID_INPUT If bk struct pointer was NULL
 * - SD_SUCCESS on success.
 *
 * \param move Allocated move struct pointer.
 * \return SD_SUCCESS or errorcode.
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
 * Return values:
 * - SD_OUT_OF_MEMORY If memory ran out. Destination struct should now be considered invalid and freed.
 * - SD_INVALID_INPUT Either input value was NULL.
 * - SD_SUCCESS on success. 
 *
 * \param dst Destination move struct pointer.
 * \param src Source move struct pointer.
 * \return SD_SUCCESS or errorcode.
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
 * Return values:
 * - SD_OUT_OF_MEMORY Memory ran out. Animation field will be NULL.
 * - SD_INVALID_INPUT Move struct pointer was NULL.
 * - SD_SUCCESS on success. 
 *
 * \param move Move struct to modify.
 * \param animation Animation to set. This will be copied.
 * \return SD_SUCCESS or errorcode.
 */ 
int sd_move_set_animation(sd_move *move, const sd_animation *animation);

/*! \brief Get the current animation
 *
 * Returns a pointer to the current animation for the move. If animation
 * is not set, NULL will be returned.
 *
 * Return values:
 * - NULL will be returned if animation does not exist
 * - sd_animation pointer will be returned on success
 *
 * \param move Move struct to modify.
 * \return sd_animation pointer on success or NULL on error.
 */ 
sd_animation* sd_move_get_animation(const sd_move *move);

/*! \brief Set move footer string for the Move struct.
 *
 * Sets the move footer string for the Move struct. Maximum length is 
 * 512 bytes. Longer strings will result in error.
 *
 * Return values:
 * - SD_INVALID_INPUT Input string was too long.
 * - SD_SUCCESS on success. 
 *
 * \param move Move struct to modify.
 * \param str String to set.
 * \return SD_SUCCESS or errorcode.
 */ 
int sd_move_set_footer_string(sd_move *move, const char *str);

/*! \brief Set move string for the Move struct.
 *
 * Sets the move string for the Move struct. Maximum length is 
 * 21 bytes. Longer strings will result in error.
 *
 * Return values:
 * - SD_INVALID_INPUT Input string was too long.
 * - SD_SUCCESS on success. 
 *
 * \param move Move struct to modify.
 * \param str String to set.
 * \return SD_SUCCESS or errorcode.
 */ 
int sd_move_set_move_string(sd_move *move, const char *str);

#ifdef SD_USE_INTERNAL
int sd_move_load(sd_reader *reader, sd_move *move);
int sd_move_save(sd_writer *writer, const sd_move *move);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_MOVE_H
