/*! \file
 * \brief Scoreboard data handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 Scoreboard files (SCORES.DAT).
 * \copyright MIT license.
 * \date 2013-2014
 * \author animehunter
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_SCORE_H
#define _SD_SCORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SD_SCORE_PAGES 4    ///< Number of scoreboard pages
#define SD_SCORE_ENTRIES 20 ///< Number of scoreboard entries

/*! \brief Score entry
 *
 * A single score entry for the scoreboard.
 */
typedef struct {
    uint32_t score;      ///< Player score
    char name[16];       ///< Player name (NULL terminated)
    uint32_t har_id:6;   ///< Har ID
    uint32_t pilot_id:6; ///< Pilot ID
    uint32_t padding:20; ///< empty padding
} sd_score_entry;

/*! \brief Scoreboard scores list
 *
 * A list of scoreboard entries. Always has 4 pages:
 * - One Round
 * - Best 2 of 3
 * - Best 3 of 5
 * - Best 4 of 7
 *
 * Each page always has 20 score entries, even though some of them may be empty (score = 0).
 */
typedef struct {
    sd_score_entry scores[SD_SCORE_PAGES][SD_SCORE_ENTRIES]; ///< Scoreboard entries list.
} sd_score;

/*! \brief Initialize score structure
 *
 * Initializes the score structure with empty values.
 *
 * \retval SD_INVALID_INPUT Score struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param score Allocated score struct pointer.
 */
int sd_score_create(sd_score *score);

/*! \brief Free score structure
 *
 * Frees up all memory reserved by the score structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param score Score struct to free.
 */
void sd_score_free(sd_score *score);

/*! \brief Load score file
 *
 * Loads the given score file to memory. The structure must be initialized with sd_score_create()
 * before using this function. Loading to a previously loaded or filled sd_score structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_OUT_OF_MEMORY Memory ran out. This struct should now be considered invalid and freed.
 * \retval SD_SUCCESS Success.
 *
 * \param score Score struct pointer.
 * \param filename Name of the score file to load from.
 */
int sd_score_load(sd_score *score, const char *filename);

/*! \brief Save score file
 *
 * Saves the given score file from memory to a file on disk. The structure must be at
 * least initialized by using sd_score_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param score Score struct pointer.
 * \param filename Name of the score file to save into.
 */
int sd_score_save(const sd_score *score, const char *filename);

/*! \brief Returns a score information entry.
 *
 * Returns a pointer to a score information entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * \retval NULL If score ptr was NULL or score information entry does not exist.
 * \retval sd_score_entry* Score information struct pointer on success.
 *
 * \param score Score information struct pointer.
 * \param page Scoreboard page number.
 * \param entry_id Score information entry id.
 */
const sd_score_entry* sd_score_get(const sd_score *score, int page, int entry_id);

#ifdef __cplusplus
}
#endif

#endif // _SD_SCORE_H
