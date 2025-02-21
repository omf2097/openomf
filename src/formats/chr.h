/*! \file
 * \brief Savegame file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 savegame (CHR) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_CHR_H
#define SD_CHR_H

#include "formats/palette.h"
#include "formats/pilot.h"
#include "formats/sprite.h"
#include "formats/tournament.h"

#define MAX_CHR_ENEMIES 256 ///< Maximum amount of enemies for a CHR file.

/*! \brief CHR enemy state entry
 *
 * Contains information about the current state of an enemy in the selected tournament.
 */
typedef struct {
    sd_pilot pilot;    ///< Enemy pilot data
    char unknown_a[9]; ///< Unknown data TODO: Find out what this does
    uint8_t photo_id;
    char unknown_b[15];
} sd_chr_enemy;

/*! \brief CHR Saved game
 *
 * Contains a saved game for a single player.
 * \todo find out what the unknown data fields are.
 */
typedef struct {
    sd_pilot pilot;                         ///< Pilot data
    vga_palette pal;                        ///< Pilot palette
    uint32_t unknown_b;                     ///< Unkown value. Maybe tells if there is photo data ?
    sd_sprite *photo;                       ///< Pilot photo
    float winnings_multiplier;              ///< Tournament winnings multiplier
    sd_chr_enemy *enemies[MAX_CHR_ENEMIES]; ///< List of enemy states in current tournament
    unsigned int cutscene;                  ///< cutscene id for end of tournament
    char *cutscene_text[10];                ///< cutscene dialog text
} sd_chr_file;

/*! \brief Initialize CHR structure
 *
 * Initializes the CHR structure with empty values.
 *
 * \retval SD_INVALID_INPUT CHR struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param chr Allocated CHR struct pointer.
 */
int sd_chr_create(sd_chr_file *chr);

/*! \brief Free CHR structure
 *
 * Frees up all memory reserved by the CHR structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param chr CHR struct to modify.
 */
void sd_chr_free(sd_chr_file *chr);

int sd_chr_from_trn(sd_chr_file *chr, sd_tournament_file *trn, sd_pilot *pilot);

/*! \brief Load .CHR file
 *
 * Loads the given CHR file to memory. The structure must be initialized with sd_chr_create()
 * before using this function. Loading to a previously loaded or filled sd_chr_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR File does not contain valid data or has syntax problems.
 * \retval SD_SUCCESS Success.
 *
 * \param chr CHR struct pointer.
 * \param filename Name of the CHR file to load from.
 */
int sd_chr_load(sd_chr_file *chr, const char *filename);

/*! \brief Save .CHR file
 *
 * Saves the given CHR file from memory to a file on disk. The structure must be at
 * least initialized by using sd_chr_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param chr CHR struct pointer.
 * \param filename Name of the CHR file to save into.
 */
int sd_chr_save(sd_chr_file *chr, const char *filename);

/*! \brief Returns an enemy entry.
 *
 * Returns a pointer to a tournament enemy savestate entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * \retval NULL If chr ptr was NULL or enemy entry does not exist.
 * \retval sd_chr_enemy* Enemy entry pointer on success.
 *
 * \param chr CHR struct pointer.
 * \param enemy_num Enemy number to find
 */
const sd_chr_enemy *sd_chr_get_enemy(sd_chr_file *chr, int enemy_num);

#endif // SD_CHR_H
