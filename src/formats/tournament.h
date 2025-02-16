/*! \file
 * \brief Tournament handling
 * \details Functions and structs for reading, writing and modifying OMF:2097 tournament (TRN) files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_TOURNAMENT_H
#define SD_TOURNAMENT_H

#include "formats/palette.h"
#include "formats/pilot.h"
#include "formats/sprite.h"

#define MAX_TRN_ENEMIES 256 ///< Maximum amount of tournament enemies
#define MAX_TRN_LOCALES 10  ///< Maximum amount of tournament locales (some of these are unused)

/*! \brief Locales
 *
 * List of tournament locales. Many of these are actually unused.
 */
enum
{
    TRN_LANG_ENGLISH = 0,
    TRN_LANG_GERMAN,
    TRN_LANG_FRENCH,
    TRN_LANG_SPANISH,
    TRN_LANG_MEXICAN,
    TRN_LANG_ITALIAN,
    TRN_LANG_POLISH,
    TRN_LANG_RUSSIAN,
    TRN_LANG_UNDEF_1,
    TRN_LANG_UNDEF_2
};

/*! \brief Tournament locale
 *
 * Translated resources for the tournament.
 */
typedef struct {
    sd_sprite *logo;            ///< Tournament logo
    char *title;                ///< Tournament title (eg. World Championship)
    char *description;          ///< Tournament description; A short text about the tournament.
    char *stripped_description; ///< Tournament description stripped of metadata.
    int desc_width;             ///< Tournament description width.
    int desc_center;            ///< Tournament description center.
    int desc_vmove;             ///< Tournament description vmove.
    int desc_size;              ///< Tournament description size.
    int desc_color;             ///< Tournament description color.
    char *end_texts[11][10];    ///< Tournament victory texts that are visible at the ending.
} sd_tournament_locale;

/*! \brief Tournament data
 *
 * Tournament enemies, locales, quotes, name, etc.
 */
typedef struct {
    char filename[14];
    uint32_t enemy_count;          ///< Number of enemies in tournament
    char bk_name[14];              ///< Tournament BK filename
    float winnings_multiplier;     ///< Match winnings multiplier
    int32_t unknown_a;             ///< Unknown /todo find out
    int32_t registration_fee;      ///< Tournament registration fee
    int32_t assumed_initial_value; ///< Value the player is assumed to have reached, minus starting value, when entering
                                   ///< this tournament.
    int32_t tournament_id;         ///< ID for the tournament
    char *pic_file;                ///< Tournament PIC filename

    sd_pilot *enemies[MAX_TRN_ENEMIES];             ///< List of enemy pilots
    sd_tournament_locale *locales[MAX_TRN_LOCALES]; ///< List of locales. If locale does not exist, it is NULL.

    vga_palette pal; ///< Tournament palette
} sd_tournament_file;

/*! \brief Initialize TRN file structure
 *
 * Initializes the TRN file structure with empty values.
 *
 * \retval SD_INVALID_INPUT TRN struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param trn Allocated TRN struct pointer.
 */
int sd_tournament_create(sd_tournament_file *trn);

/*! \brief Load a TRN file
 *
 * Loads the given TRN file to memory. The structure must be initialized with sd_trn_create()
 * before using this function. Loading to a previously loaded or filled sd_tournament_file structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_FILE_PARSE_ERROR Syntax error in file.
 * \retval SD_SUCCESS Success.
 *
 * \param trn TRN file struct pointer.
 * \param filename Name of the TRN file to load from.
 */
int sd_tournament_load(sd_tournament_file *trn, const char *filename);

/*! \brief Save TRN file
 *
 * Saves the given TRN file from memory to a file on disk. The structure must be at
 * least initialized by using sd_trn_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param trn TRN file struct pointer.
 * \param filename Name of the TRN file to save into.
 */
int sd_tournament_save(const sd_tournament_file *trn, const char *filename);

int sd_tournament_set_bk_name(sd_tournament_file *trn, const char *bk_name);
int sd_tournament_set_pic_name(sd_tournament_file *trn, const char *pic_name);

/*! \brief Free TRN file structure
 *
 * Frees up all memory reserved by the TRN structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param trn TRN file struct pointer.
 */
void sd_tournament_free(sd_tournament_file *trn);

#endif // SD_TOURNAMENT_H
