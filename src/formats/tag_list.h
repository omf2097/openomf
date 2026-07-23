/**
 * @file tag_list.h
 * @brief Animation tag list handling.
 * @details Animation tag information fetching.
 * @copyright MIT License
 * @date 2013-2026
 * @author animehunter
 * @author Tuomas Virtanen
 */

#ifndef TAG_LIST_H
#define TAG_LIST_H

/** @brief Tag information entry
 *
 * Contains information about a single animation tag.
 */
typedef struct {
    const char *tag;         ///< Tag string
    const int has_param;     ///< Tells if the tag can be expected to have a parameter.
    const char *description; ///< A short description for the tag.
} tag_descriptor;

extern const tag_descriptor tag_descriptor_list[]; ///< A global list of tags
extern const int tag_descriptor_count;             ///< Tag list size

#endif // TAG_LIST_H
