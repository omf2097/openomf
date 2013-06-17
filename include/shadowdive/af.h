/*! \file 
 * \brief Contains stuff that handles AF files.
 * \license MIT
 */ 

#ifndef _SD_AF_H
#define _SD_AF_H

#include <stdint.h>

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef _SD_MOVE_H
typedef struct sd_move_t sd_move;
#endif

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

    sd_move *moves[70];

    char soundtable[30];
} sd_af_file;

/*! \brief Create AF container
 * Creates the AF container struct, allocated memory etc.
 * \return AF container struct pointer
 */
sd_af_file* sd_af_create();

/*! \brief Load AF file
 * Loads the given AF file to memory
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param filename Name of the AF file.
 * \return SD_SUCCESS on success.
 */
int sd_af_load(sd_af_file *af, const char *filename);

/*! \brief Save AF file
 * Saves the given AF file from memory to a file on disk.
 * \param af AF struct pointer. Must be created using sd_af_create().
 * \param filename Name of the AF file to save into.
 * \return SD_SUCCESS on success.
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
