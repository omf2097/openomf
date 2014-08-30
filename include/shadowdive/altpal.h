/*! \file
 * \brief Alternate palette  file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 alternate palette file (ALTPALS.DAT).
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */ 

#ifndef _SD_ALTPAL_H
#define _SD_ALTPAL_H

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_ALTPALS_PALETTES 11 ///< Maximum amount of alternate palettes (technical limitation)

/*! \brief Alternate palettes
 *
 * A simple list of alternate palettes.
 */
typedef struct {
    sd_palette palettes[SD_ALTPALS_PALETTES]; ///< List of palettes
} sd_altpal_file;

/*! \brief Initialize the alternate palette list structure
 *
 * Initializes the alternate palette list structure with empty values.
 *
 * \retval SD_INVALID_INPUT Alternate palette struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param ap Altpal struct pointer
 */
int sd_altpal_create(sd_altpal_file *ap);

/*! \brief Load altpals structure from file.
 * 
 * Loads the given file to memory. The structure must be initialized with sd_altpal_create() 
 * before using this function. Loading to a previously loaded or filled sd_altpal_file structure 
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * \retval SD_SUCCESS Success.
 *
 * \param ap Altpal struct pointer
 * \param filename Filename to load from.
 */
int sd_altpals_load(sd_altpal_file *ap, const char *filename);

/*! \brief Save altpals structure to file.
 * 
 * Saves the given altpal file from memory to a file on disk. The structure must be at
 * least initialized by using sd_altpal_create() before running this.
 * 
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 * 
 * \param ap Altpal struct pointer
 * \param filename Filename to save into.
 */
int sd_altpals_save(sd_altpal_file *ap, const char *filename);

/*! \brief Free alternate palette structure
 * 
 * Frees up all memory reserved by the structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param ap Altpal struct pointer
 */
void sd_altpal_free(sd_altpal_file *ap);

#ifdef __cplusplus
}
#endif

#endif // _SD_ALTPAL_H
