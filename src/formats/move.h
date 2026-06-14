/**
 * @file move.h
 * @brief AF move handling.
 * @details Functions and structs for reading, writing and modifying OMF:2097 Fighter specific move structures.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef SD_MOVE_H
#define SD_MOVE_H

#include "formats/animation.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "utils/str.h"
#include <stdint.h>

#define SD_MOVE_STRING_MAX 21         ///< Maximum allowed move string length
#define SD_MOVE_FOOTER_STRING_MAX 512 ///< Maximum allowed footer string length

/** @brief AI move trigger bits (the ai_flags field).
 *
 * Each bit marks the move as relevant for a given opponent state / AI stance.
 */
enum
{
    AI_NONE = 0x0000,
    AI_GND_STATE_1 = 0x0001,        ///< Opponent grounded, AI stance slot 1
    AI_VS_HIGH_OR_IDLE = 0x0002,    ///< Opponent high attack or idle
    AI_VS_LOW_OR_CROUCH = 0x0004,   ///< Opponent low attack or crouching
    AI_VS_MEDIUM = 0x0008,          ///< Opponent medium attack
    AI_VS_PROJECTILE_HIGH = 0x0010, ///< Opponent projectile at or above the Y threshold
    AI_GND_STATE_2 = 0x0020,        ///< Opponent grounded, AI stance slot 2
    AI_GND_STATE_3 = 0x0040,        ///< Opponent grounded, AI stance slot 3
    AI_GND_STATE_0 = 0x0080,        ///< Opponent grounded, AI stance slot 0
    AI_AIR_STATE_0 = 0x0100,        ///< Opponent airborne, AI stance slot 0
    AI_AIR_STATE_1 = 0x0200,        ///< Opponent airborne, AI stance slot 1
    AI_AIR_STATE_2 = 0x0400,        ///< Opponent airborne, AI stance slot 2
    AI_AIR_STATE_3 = 0x0800,        ///< Opponent airborne, AI stance slot 3
    AI_FLAG_1000 = 0x1000,          ///< Unknown
    AI_VS_PROJECTILE_LOW = 0x2000,  ///< Opponent projectile below the Y threshold
    AI_FLAG_4000 = 0x4000,          ///< Reserved
    AI_FLAG_8000 = 0x8000,          ///< Reserved
};

/** @brief Position and state requirement bits (the pos_constraint field).
 *
 * Every bit must be satisfied by the HAR's current state for the move to be
 * allowed.
 */
enum
{
    POS_NONE = 0x0000,
    POS_WALL = 0x0001,                  ///< HAR is near a wall
    POS_NO_DIST_CHECK = 0x0002,         ///< Bypass distance and position gates
    POS_JUMP_WALL_BOUNCE = 0x0004,      ///< Aerial hit triggers wall-bounce on the target
    POS_IN_ARENA0 = 0x0008,             ///< Require arena 0
    POS_IN_ARENA1 = 0x0010,             ///< Require arena 1
    POS_IN_ARENA2 = 0x0020,             ///< Require arena 2
    POS_IN_ARENA3 = 0x0040,             ///< Require arena 3 (Fire Pit)
    POS_IN_ARENA4 = 0x0080,             ///< Require arena 4
    POS_FLAG_100 = 0x0100,              ///< Reserved
    POS_FLAG_200 = 0x0200,              ///< Reserved
    POS_FLAG_400 = 0x0400,              ///< Reserved
    POS_FLAG_800 = 0x0800,              ///< Reserved
    POS_FLAG_1000 = 0x1000,             ///< Reserved
    POS_FIRE_ICE_ALLOWED = 0x2000,      ///< Fire/ice move is currently allowed
    POS_OPP_FIRE_ICE_DESTRUCT = 0x4000, ///< Opponent is a special destruct target
    POS_FLAG_8000 = 0x8000,             ///< Reserved
};

/** @brief Extra string selector index. */
enum
{
    ESS_NONE = 0,        ///< No stat scaling, mixed string
    ESS_ARM_SPEED = 1,   ///< Arm stat
    ESS_LEG_SPEED = 2,   ///< Leg stat
    ESS_SPECIAL_ARM = 3, ///< Arm stat, string chosen by enhancement level
    ESS_SPECIAL_LEG = 4, ///< Leg stat, string chosen by enhancement level
    ESS_SPECIAL = 5,     ///< Both arm and leg stats
};

/** @brief HAR Move information
 *
 * Contains information about the HAR move. Wraps a generic animation.
 * @todo Find out what the unknown fields are.
 */
typedef struct {
    sd_animation *animation; ///< Animation field for Move. When saving AF file, this should be != NULL.

    uint16_t ai_flags;       ///< AI move trigger bitmask (AI_* enum)
    uint16_t pos_constraint; ///< Position and state requirement bitmask (PC_* enum)
    uint8_t unknown_4;       ///< Unknown value
    uint8_t unknown_5;       ///< Unknown value
    uint8_t unknown_6;       ///< Unknown value
    uint8_t unknown_7;       ///< Unknown value
    uint8_t unknown_8;       ///< Unknown value
    uint8_t unknown_9;       ///< Unknown value
    uint8_t unknown_10;      ///< Unknown value
    uint8_t unknown_11;      ///< Unknown value
    uint8_t play_if_hit;     ///< Animation to chain into when this move connects
    uint8_t category;        ///< Move category ID
    uint8_t block_damage;    ///< Damage applied when blocking this mode
    uint8_t block_stun;   ///< How many frames to force the opponents block animation on hit, also used for scrap amount
    uint8_t successor_id; ///< Successor animation ID
    uint8_t damage_amount;         ///< Damage amount when this move connects
    uint8_t throw_duration;        ///< How many frames the HAR is locked into a throw
    uint8_t extra_string_selector; ///< Animation string variant selector (ESS_* enum)
    uint8_t points;                ///< Score gained for this hit

    str move_string;   ///< Move string
    str footer_string; ///< Footer string
} sd_move;

/** @brief Initialize Move structure
 *
 * Initializes the move structure with empty values.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param move Allocated move struct pointer.
 */
int sd_move_create(sd_move *move);

/** @brief Copy Move structure
 *
 * Copies the contents of an move structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_move_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * move structure, or problems are likely to appear.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param dst Destination move struct pointer.
 * @param src Source move struct pointer.
 */
int sd_move_copy(sd_move *dst, const sd_move *src);

/** @brief Free move structure
 *
 * Frees up all memory reserved by the move structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * @param move Move struct to modify.
 */
void sd_move_free(sd_move *move);

/** @brief Set animation struct for move
 *
 * Sets an animation for the move. Animation will be copied,
 * so remember to free your local copy yourself. Note that any valid
 * move struct should ALWAYS contain an animation. Otherwise there will be problems
 * eg. saving the AF file.
 *
 * A NULL value for animation field will result in move->animation field getting freed.
 *
 * @retval SD_SUCCESS Success.
 *
 * @param move Move struct to modify.
 * @param animation Animation to set. This will be copied.
 */
int sd_move_set_animation(sd_move *move, const sd_animation *animation);

/** @brief Get the current animation
 *
 * Returns a pointer to the current animation for the move. If animation
 * is not set, NULL will be returned.
 *
 * @retval NULL Animation does not exist
 * @retval sd_animation* Success
 *
 * @param move Move struct to modify.
 */
sd_animation *sd_move_get_animation(const sd_move *move);

/** @brief Load Move from an open reader
 *
 * @retval SD_FILE_PARSE_ERROR File does not contain valid data.
 * @retval SD_SUCCESS Success.
 *
 * @param reader Open reader to read from.
 * @param move Move struct to fill.
 */
int sd_move_load(sd_reader *reader, sd_move *move);

/** @brief Save Move to an open writer
 *
 * @retval SD_SUCCESS Success.
 *
 * @param writer Open writer to write to.
 * @param move Move struct to save.
 */
int sd_move_save(sd_writer *writer, const sd_move *move);

#endif // SD_MOVE_H
