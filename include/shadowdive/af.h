/*! \file 
 * \brief Contains functions for handling .AF (HAR) files.
 * \license MIT
 */ 

#ifndef _SD_AF_H
#define _SD_AF_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <stdint.h>

#ifndef _SD_MOVE_H
typedef struct sd_move_t sd_move;
#endif

// This is hardcoded, do not change (unless you want to change the file format, of course).
#define MAX_AF_MOVES 70

typedef struct sd_af_file_t {
    uint16_t file_id;
    uint16_t unknown_a;
    uint32_t endurance;
    uint8_t unknown_b;
    uint16_t power;
    int32_t forward_speed;
    int32_t reverse_speed;
    int32_t jump_speed;
    int32_t fall_speed;
    uint16_t unknown_c;

    sd_move *moves[MAX_AF_MOVES];
    char soundtable[30];
} sd_af_file;

/*! \brief Create AF container
 * Creates the AF container struct, allocated memory etc.
 * \return AF container struct pointer
 */
sd_af_file* sd_af_create();

/*! \ brief Set move
 * Sets a HAR move to a given index in a AF file structure.
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param index Index of the move. Must be 0 <= index <= 69.
 * \param move Sd_move pointer will be copied, so do not free the sd_move after setting it!
 */ 
void sd_af_set_move(sd_af_file *af, int index, sd_move *move);

/*! \ brief Get move
 * Gets a HAR move from an index in AF file structure
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param index Index of the move. Must be 0 <= index <= 69.
 * \return sd_move pointer in success, or NULL in failure.
 */ 
sd_move* sd_af_get_move(sd_af_file *af, int index);

/*! \brief Load AF file
 * Loads the given AF file to memory. Note! Do not load twice to the same AF structure.
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param filename Name of the AF file.
 * \return SD_SUCCESS on success, or SD_FILE_OPEN_ERROR or SD_FILE_PARSE_ERROR on failure.
 */
int sd_af_load(sd_af_file *af, const char *filename);

/*! \brief Save AF file
 * Saves the given AF file from memory to a file on disk.
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param filename Name of the AF file to save into.
 * \return SD_SUCCESS on success, or SD_FILE_OPEN_ERROR on failure.
 */
int sd_af_save(sd_af_file *af, const char* filename);

/*! \brief Free AF container
 * Frees up the af struct memory.
 * \param af AF struct pointer. Must be created using sd_af_create().
 */
void sd_af_delete(sd_af_file *af);

#ifdef __cplusplus
}
#endif

#endif // _SD_AF_H
