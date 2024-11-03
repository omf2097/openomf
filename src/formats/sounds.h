/*! \file
 * \brief Sounds handling
 * \details Functions and structs for reading, writing and modifying OMF:2097 sound files (SOUNDS.DAT).
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_SOUNDS_H
#define SD_SOUNDS_H

#include <inttypes.h>

#define SD_SOUNDS_MAX 299 ///< Maximum amount of sounds allowed in the SOUNDS.DAT file.

/*! \brief A single sound entry.
 *
 * Format:
 * - 8bit unsigned
 * - 8000Hz (can be changed by animation tags)
 * - Mono
 */
typedef struct {
    uint16_t len; ///< Sound length in bytes
    char *data;   ///< Sound data
    uint8_t unknown;
} sd_sound;

/*! \brief Sounds list
 *
 * A simple list of sounds.
 */
typedef struct {
    sd_sound sounds[SD_SOUNDS_MAX]; ///< Sounds list
} sd_sound_file;

/*! \brief Initialize sounds structure
 *
 * Initializes the sounds structure with empty values.
 *
 * \retval SD_INVALID_INPUT Score struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param sf Allocated sounds struct pointer.
 */
int sd_sounds_create(sd_sound_file *sf);

/*! \brief Free sounds structure
 *
 * Frees up all memory reserved by the sounds structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param sf Sounds struct to free.
 */
void sd_sounds_free(sd_sound_file *sf);

/*! \brief Returns a sound entry.
 *
 * Returns a pointer to a sound entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * \retval NULL If sd_sound_file ptr was NULL or sound does not exist.
 * \retval sd_sound* Sound entry pointer on success.
 *
 * \param sf Sound information struct pointer.
 * \param id Sound identifier (0 - 299).
 */
const sd_sound *sd_sounds_get(const sd_sound_file *sf, int id);

/*! \brief Save a sound to an AU file.
 *
 * Saves a 8bit, mono, unsigned, 8000Hz PCM sample to an AU file.
 *
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param sf Source sounds pointer
 * \param num Sound ID to export to
 * \param filename Destination filename
 */
int sd_sound_to_au(const sd_sound_file *sf, int num, const char *filename);

/*! \brief Load a sound from an AU file.
 *
 * Loads a 8bit, mono, unsigned, 8000Hz PCM sample from an AU file.
 * If given sound number already has a sound, it will be freed.
 *
 * \retval SD_INVALID_INPUT Image or filename was NULL
 * \retval SD_FILE_INVALID_TYPE Input image was of invalid type.
 * \retval SD_FILE_OPEN_ERROR File could not be opened for reading.
 * \retval SD_SUCCESS Success.
 *
 * \param sf Destination sounds pointer
 * \param num Sound ID to import
 * \param filename Source filename
 */
int sd_sound_from_au(sd_sound_file *sf, int num, const char *filename);

/*! \brief Load sounds file
 *
 * Loads the given sounds file to memory. The structure must be initialized with sd_sounds_create()
 * before using this function. Loading to a previously loaded or filled sd_sound_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_SUCCESS Success.
 *
 * \param sf Sounds struct pointer.
 * \param filename Name of the sounds file to load from.
 */
int sd_sounds_load(sd_sound_file *sf, const char *filename);

/*! \brief Save sounds file
 *
 * Saves the given sounds file from memory to a file on disk. The structure must be at
 * least initialized by using sd_sounds_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param sf Sounds struct pointer.
 * \param filename Name of the sounds file to save into.
 */
int sd_sounds_save(const sd_sound_file *sf, const char *filename);

#endif // SD_SOUNDS_H
