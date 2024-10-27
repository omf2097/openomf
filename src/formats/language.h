/*! \file
 * \brief Language file handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 language files.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef SD_LANGUAGE_H
#define SD_LANGUAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Language string container
 *
 * Contains a single language string and a short description for it. Descriptions
 * may or may not be accurate.
 */
typedef struct {
    char description[32]; ///< Language string short description
    char *data;           ///< Language string itself.
} sd_lang_string;

/*! \brief Language string list
 *
 * Contains a list of language string descriptors.
 */
typedef struct {
    unsigned int count;      ///< Amount of language strings in the file
    sd_lang_string *strings; ///< Language string array
} sd_language;

/*! \brief Initialize language structure
 *
 * Initializes the language structure with empty values.
 *
 * \retval SD_INVALID_INPUT Language struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param language Allocated language struct pointer.
 */
int sd_language_create(sd_language *language);

/*! \brief Free language structure
 *
 * Frees up all memory reserved by the language structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param language Language struct to modify.
 */
void sd_language_free(sd_language *language);

/*! \brief Load a language file
 *
 * Loads the given language file to memory. The structure must be initialized with sd_language_create()
 * before using this function. Loading to a previously loaded or filled sd_language structure
 * will result in old data and pointers getting lost. This is very likely to cause a memory leak.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened.
 * \retval SD_SUCCESS Success.
 *
 * \param language Language struct pointer.
 * \param filename Name of the language file to load from.
 */
int sd_language_load(sd_language *language, const char *filename);

/*! \brief Save language file
 *
 * Saves the given language file from memory to a file on disk. The structure must be at
 * least initialized by using sd_language_create() before running this.
 *
 * \retval SD_FILE_OPEN_ERROR File could not be opened for writing.
 * \retval SD_SUCCESS Success.
 *
 * \param language Language struct pointer.
 * \param filename Name of the language file to save into.
 */
int sd_language_save(sd_language *language, const char *filename);

/*! \brief Returns a language string entry.
 *
 * Returns a pointer to a language string entry.
 *
 * The structure memory will be owned by the library; do not attempt to
 * free it.
 *
 * \retval NULL If language ptr was NULL or string entry does not exist.
 * \retval sd_lang_string* Language string struct pointer on success.
 *
 * \param language Language struct pointer.
 * \param num Language entry number to get.
 */
const sd_lang_string *sd_language_get(const sd_language *language, int num);

void sd_language_append(sd_language *language, const char *description, const char *data);

#ifdef __cplusplus
}
#endif

#endif // SD_LANGUAGE_H
