/*! \file
 * \brief Animation tag list handling.
 * \details Animation tag information fetching.
 * \copyright MIT license.
 * \date 2013-2014
 * \author animehunter
 * \author Tuomas Virtanen
 */

#ifndef SD_TAGLIST_H
#define SD_TAGLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Tag information entry
 *
 * Contains information about a single animation tag.
 */
typedef struct {
    const char *tag;         ///< Tag string
    const int has_param;     ///< Tells if the tag can be expected to have a parameter.
    const char *description; ///< A short description for the tag.
} sd_tag;

extern const sd_tag sd_taglist[]; ///< A global list of tags
extern const int sd_taglist_size; ///< Taglist size

/*! \brief Fetch information about a tag
 *
 * Returns information about a single tag. On success, req_param, tag and desc
 * parameters will be set. Any of the parameters can be ignored by setting the parameter
 * value to NULL on function call.
 *
 * \retval SD_INVALID_INPUT Tag does not exist.
 * \retval SD_SUCCESS Success.
 *
 * \param search_tag A Tag to look for
 * \param req_param Tells if the tag requires a parameter. Will be ignored if set to NULL.
 * \param tag A pointer to the tag string in library memory. Will be ignored if set to NULL.
 * \param desc A pointer to the description in library memory. Will be ignored if set to NULL.
 */
int sd_tag_info(const char *search_tag, int search_len, int *req_param, const char **tag, const char **desc);

#ifdef __cplusplus
}
#endif

#endif // SD_TAGLIST_H
