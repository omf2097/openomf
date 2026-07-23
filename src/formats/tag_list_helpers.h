/**
 * @file tag_list_helpers.h
 * @brief Animation tag list helper functions.
 * @details Hand written helpers operating on the generated tag list.
 * @copyright MIT License
 * @date 2013-2026
 * @author Tuomas Virtanen
 */

#ifndef TAG_LIST_HELPERS_H
#define TAG_LIST_HELPERS_H

#include "formats/script.h"
#include "formats/script_tag.h"
#include <stdbool.h>

/** @brief Map a tag string slice to its enum
 * @param buf Buffer containing the tag.
 * @param len Length of the tag slice.
 * @param tag A pointer that receives the matching enum. Will be ignored if set to NULL.
 * @return true if a tag matches the slice, false otherwise.
 */
bool script_tag_lookup(const char *buf, int len, script_tag *tag);

/** @brief Returns the single character name of an unknown tag
 * @param ch The original character stored for the unknown tag.
 * @return The name string or NULL
 */
const char *script_invalid_tag_name(char ch);

/** @brief Returns the tag name string
 * @param tag Tag to inspect
 * @return The tag string or NULL
 */
const char *script_get_frame_tag_name(const script_frame_tag *tag);

/** @brief Returns the tag description string
 * @param tag Tag to inspect
 * @return The tag description or NULL
 */
const char *script_get_frame_tag_description(const script_frame_tag *tag);

#endif // TAG_LIST_HELPERS_H
