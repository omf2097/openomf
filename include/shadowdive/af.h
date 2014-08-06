/*! \file 
 * \brief Contains functions for handling .AF (HAR) files.
 * \license MIT
 */ 

#ifndef _SD_AF_H
#define _SD_AF_H

#include <stdint.h>
#include "shadowdive/move.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define MAX_AF_MOVES 70

typedef struct {
    uint16_t file_id;
    uint16_t unknown_a;
    uint32_t endurance;
    uint8_t unknown_b;
    uint16_t power;
    int32_t forward_speed;
    int32_t reverse_speed;
    int32_t jump_speed;
    int32_t fall_speed;
    uint8_t unknown_c;
    uint8_t unknown_d;

    sd_move moves[MAX_AF_MOVES];
    char soundtable[30];
} sd_af_file;

/*! \brief Initialize AF container
 * Initializes the AF container with empty values. When saved, this will create an empty AF file.
 * \param af Allocated AF struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_af_create(sd_af_file *af);

/*! \brief Copy AF structure
 * Copies a valid AF structure. Note: Source structure must be valid.
 * \param dst Destination AF struct pointer.
 * \param src Source AF struct pointer.
 * \return SD_SUCCESS or errorcode.
 */
int sd_af_copy(sd_af_file *dst, const sd_af_file *src);

/*! \ brief Set move
 * Sets a HAR move to a given index in a AF file structure.
 * \param af AF struct pointer.
 * \param index Index of the move.
 * \param move sd_move struct. This will be copied.
 * \return SD_SUCCESS or errorcode.
 */ 
int sd_af_set_move(sd_af_file *af, int index, const sd_move *move);

/*! \ brief Get move
 * Gets a HAR move from an index in AF file structure.
 * \param af AF struct pointer.
 * \param index Index of the move. Must be 0 <= index <= 69.
 * \return sd_move pointer in success, or NULL if failure.
 */ 
sd_move* sd_af_get_move(sd_af_file *af, int index);

/*! \brief Load AF file
 * Loads the given AF file to memory. The structure should be initialized with sd_af_create() before this.
 * \param af AF struct pointer.
 * \param filename Name of the AF file.
 * \return SD_SUCCESS or errorcode.
 */
int sd_af_load(sd_af_file *af, const char *filename);

/*! \brief Save AF file
 * Saves the given AF file from memory to a file on disk.
 * \param af AF struct pointer.
 * \param filename Name of the AF file to save into.
 * \return SD_SUCCESS or errorcode.
 */
int sd_af_save(const sd_af_file *af, const char* filename);

/*! \brief Free AF container
 * Frees up all memory reserved by the AF container. All contents will be freed, all pointers to contents will be invalid.
 * \param af AF struct pointer.
 */
void sd_af_free(sd_af_file *af);

#ifdef __cplusplus
}
#endif

#endif // _SD_AF_H
