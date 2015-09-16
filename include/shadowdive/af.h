/*! \file
 * \brief Fighter file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 fighter (AF) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_AF_H
#define _SD_AF_H

#include <stdint.h>
#include "shadowdive/move.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_AF_MOVES 70 ///< Maximum amount of moves for a HAR

/*! \brief HAR data container
 *
 * Contains information about a single HAR (combat robot).
 */
typedef struct {
    uint16_t file_id;  ///< File ID
    uint16_t exec_window; ///< Move execution window (?)
    float endurance; ///< HAR Endurance
    uint8_t unknown_b; ///< Unknown value
    uint16_t health; ///< HAR Health
    float forward_speed; ///< HAR fwd speed
    float reverse_speed; ///< HAR bwd speed
    float jump_speed; ///< HAR jump speed
    float fall_speed; ///< HAR fall speed
    uint8_t unknown_c; ///< Unknown value
    uint8_t unknown_d; ///< Unknown value

    sd_move *moves[MAX_AF_MOVES]; ///< All HAR moves.
    char soundtable[30]; ///< All sounds used by the animations in this HAR file.
} sd_af_file;

/*! \brief Initialize AF file structure
 *
 * Initializes the AF file structure with empty values.
 *
 * \retval SD_INVALID_INPUT AF struct value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param af Allocated AF struct pointer.
 */
int sd_af_create(sd_af_file *af);

/*! \brief Copy AF structure
 *
 * Copies the contents of an AF file structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_af_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * AF file structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination AF struct pointer.
 * \param src Source AF struct pointer.
 */
int sd_af_copy(sd_af_file *dst, const sd_af_file *src);

/*! \brief Set move
 *
 * Sets a HAR move to a given index in a AF file structure. Move will be copied,
 * so remember to free your local copy yourself.
 *
 * Index should be between 0 and 69 (inclusive); any other values will result in error.
 *
 * A NULL value for move will result in the old move at index getting freed.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. This struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Index was invalid or a pointer was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param af AF struct pointer.
 * \param index Index of the move. Must be 0 <= index <= 69.
 * \param move sd_move struct. This will be copied.
 */
int sd_af_set_move(sd_af_file *af, int index, const sd_move *move);

/*! \brief Get move
 *
 * Gets a HAR move from an index in AF file structure. Index must be
 * between values 0 and 69 (inclusive); any other value will result in error.
 *
 * \retval NULL Wrong index or move does not exist.
 * \retval sd_move* Success.
 *
 * \param af AF struct pointer.
 * \param index Index of the move. Must be 0 <= index <= 69.
 */
sd_move* sd_af_get_move(sd_af_file *af, int index);

/*! \brief Load AF file
 *
 * Loads the given AF file to memory. The structure must be initialized with sd_af_create()
 * before using this function.´Loading to a previously loaded or filled sd_bk_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_OUT_OF_MEMORY Memory ran out. This struct should now be considered invalid and freed.
 * \retval SD_SUCCESS Success.
 *
 * \param af AF struct pointer.
 * \param filename Name of the AF file.
 */
int sd_af_load(sd_af_file *af, const char *filename);

/*! \brief Save AF file
 *
 * Saves the given AF file from memory to a file on disk. The structure must be
 * at least initialized by using sd_af_create before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_SUCCESS Success.
 *
 * \param af AF struct pointer.
 * \param filename Name of the AF file
 */
int sd_af_save(const sd_af_file *af, const char* filename);

/*! \brief Free AF file structure
 *
 * Frees up all memory reserved by the AF file structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param af AF struct pointer.
 */
void sd_af_free(sd_af_file *af);

#ifdef __cplusplus
}
#endif

#endif // _SD_AF_H
