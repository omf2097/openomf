/*! \file
 * \brief Match record file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 match record (REC) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_REC_H
#define _SD_REC_H

#include <stdint.h>
#include <stddef.h>
#include "shadowdive/pilot.h"
#include "shadowdive/palette.h"
#include "shadowdive/sprite.h"
#include "shadowdive/actions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief REC action record
 *
 * AA record of a single action during the match.
 * Essentially a record of keys pressed at a given tick.
 */
typedef struct {
    uint32_t tick;      ///< Game tick at the moment of this event
    uint8_t lookup_id;  ///< Extra content id. Valid values 2,3,5,6,10,18.
    uint8_t player_id;  ///< Player ID. 0 or 1.
    sd_action action;   ///< Player actions at this tick. A Combination of sd_rec_action enums.
    uint8_t raw_action; ///< Raw action data from the file.
    char *extra_data;   ///< Extra data. Check length using sd_rec_extra_len(). NULL if does not exist.
} sd_rec_move;

/*! \brief REC pilot container
 *
 * Information about a single pilot in the match.
 * \todo Find out about the unknowns here
 */
typedef struct {
    sd_pilot info;       ///< Pilot information
    uint8_t unknown_a;   ///< Unknown value \todo: Find this out
    uint16_t unknown_b;  ///< Unknown value \todo: Find this out
    sd_palette pal;      ///< Palette for this pilot photo
    uint8_t has_photo;   ///< Tells if the pilot has a photo sprite
    sd_sprite photo;     ///< Photo sprite
} sd_rec_pilot;

/*! \brief REC recording
 *
 * Contains a record of a single OMF:2097 match. This may be
 * a network match, tournament match, singleplayer match ...
 * The data varies slightly depending on the type.
 * \todo Find out about the unknowns. Possibly hyper mode, etc.
 */
typedef struct {
    sd_rec_pilot pilots[2]; ///< Information about the pilots
    uint32_t scores[2];     ///< Score data at the start of the match
    int8_t unknown_a;       ///< Is Fire or ice ? 0 = no, 1 = fire, 2 = ice ?
    int8_t unknown_b;       ///< Unknown \todo: Find out
    int8_t unknown_c;       ///< Unknown \todo: Find out

    int16_t throw_range;    ///< Throw range (%)
    int16_t hit_pause;      ///< Hit pause (ticks)
    int16_t block_damage;   ///< Block damage (%)
    int16_t vitality;       ///< Vitality (%)
    int16_t jump_height;    ///< Jump height (%)
    int16_t unknown_i;      ///< Unknown \todo: Find out
    int16_t unknown_j;      ///< Unknown \todo: Find out
    int16_t unknown_k;      ///< Unknown \todo: Find out

    uint8_t knock_down;     ///< Knock down (0 = None, 1 = Kicks, 2 = Punches, 3 = both)
    uint8_t rehit_mode;     ///< Rehit mode (On/Off)
    uint8_t def_throws;     ///< Def. Throws (On/Off)
    uint8_t arena_id;       ///< Arena ID
    uint8_t power[2];       ///< Power 1,2 (0-7?)
    uint8_t hazards;        ///< Hazards (On/Off)
    uint8_t round_type;     ///< Round type (0=1, 1=2/3, 2=3/5, 3=4/7)
    uint8_t unknown_l;      ///< Currently unknown \todo Find out what this does
    uint8_t hyper_mode;     ///< Hyper mode (On/Off)

    int8_t unknown_m;       ///< Unknown \todo: Find out

    unsigned int move_count; ///< How many REC event records
    sd_rec_move *moves; ///< REC event records list
} sd_rec_file;

/*! \brief Initialize REC file structure
 *
 * Initializes the REC file structure with empty values.
 *
 * \retval SD_INVALID_INPUT REC struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param rec Allocated REC struct pointer.
 */
int sd_rec_create(sd_rec_file *rec);

/*! \brief Free REC file structure
 *
 * Frees up all memory reserved by the REC structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param rec REC file struct pointer.
 */
void sd_rec_free(sd_rec_file *rec);

/*! \brief Load .REC file
 *
 * Loads the given REC file to memory. The structure must be initialized with sd_rec_create()
 * before using this function. Loading to a previously loaded or filled sd_rec_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_OUT_OF_MEMORY Memory ran out. This struct should now be considered invalid and freed.
 * \retval SD_SUCCESS Success.
 *
 * \param rec BK struct pointer.
 * \param filename Name of the BK file to load from.
 */
int sd_rec_load(sd_rec_file *rec, const char *filename);

/*! \brief Save .REC file
 *
 * Saves the given REC file from memory to a file on disk. The structure must be at
 * least initialized by using sd_rec_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param rec REC struct pointer.
 * \param filename Name of the REC file to save into.
 */
int sd_rec_save(sd_rec_file *rec, const char *filename);

/*! \brief Deletes a REC event record
 *
 * Deletes a REC event record at given position.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. The REC structure will most likely be invalid.
 * \retval SD_INVALID_INPUT Number you tried to remove does not exist, or rec was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param rec BK struct pointer.
 * \param number Record number
 */
int sd_rec_delete_action(sd_rec_file *rec, unsigned int number);

int sd_rec_extra_len(int key);

/*! \brief Inserts a REC event record
 *
 * Inserts a new event record to a given position. All contents starting from the given
 * position will be moved forwards by one entry.
 *
 * You can push new entries to the end of the list by pointing to the last+1 entry.
 *
 * Event record data will be copied. Make sure to free your local copy yourself.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. The REC structure will most likely be invalid.
 * \retval SD_INVALID_INPUT Slot you tried to insert to does not exist, or rec was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param rec BK struct pointer.
 * \param number Record number
 * \param move Move to insert
 */
int sd_rec_insert_action(sd_rec_file *rec, unsigned int number, const sd_rec_move *move);

#ifdef __cplusplus
}
#endif

#endif // _SD_REC_H
