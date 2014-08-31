/*! \file
 * \brief Animation string parser
 * \details Functions and structs for reading, writing and modifying OMF:2097 animation strings.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */ 

#ifndef _SD_SCRIPT_H
#define _SD_SCRIPT_H

#include "shadowdive/taglist.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Animation tag
 *
 * Describes a single tag in animation frame.
 */
typedef struct {
    const char* key;  ///< Tag name
    const char* desc; ///< Tag description
    int has_param;    ///< Tells if the tag has a parameter
    int value;        ///< Tag parameter value. Only valid if has_param = 1.
} sd_script_tag;

/*! \brief Animation frame
 *
 * Describes a single frame in animation string.
 */
typedef struct {
    int sprite;          ///< Sprite ID that the frame relates to
    int tick_len;        ///< Length of the frame in ticks
    int tag_count;       ///< Amount of tags in this frame
    sd_script_tag *tags; ///< A list of tags in this frame
} sd_script_frame;

/*! \brief Animation script
 *
 * A single animation string. Contains multiple frames, which then contain tags.
 * A valid string must contain at least a single frame.
 */
typedef struct {
    int frame_count;          ///< Amount of frames in the string
    sd_script_frame *frames;  ///< List of frames in this string
} sd_script;

/*! \brief Initialize script parser
 *
 * Initializes the script parser with empty values.
 *
 * \retval SD_INVALID_INPUT Script struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param script Allocated script struct pointer.
 */
int sd_script_create(sd_script *script);

/*! \brief Free script parser
 * 
 * Frees up all memory reserved by the script parser structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param script Script struct to free.
 */
void sd_script_free(sd_script *script);
int sd_script_decode(sd_script *script, const char* str, int *invalid_pos);
int sd_script_encode(const sd_script *script, char* str);
int sd_script_encoded_length(const sd_script *script);
int sd_script_get_total_ticks(const sd_script *script);

const sd_script_frame* sd_script_get_frame_at(const sd_script *script, int ticks);
const sd_script_frame* sd_script_get_frame(const sd_script *script, int frame_number);
const sd_script_tag* sd_script_get_tag(const sd_script_frame* frame, const char* tag);
int sd_script_frame_changed(const sd_script *script, int tick_start, int tick_stop);
int sd_script_get_frame_index(const sd_script *script, const sd_script_frame *frame);
int sd_script_get_frame_index_at(const sd_script *script, int ticks);
int sd_script_is_last_frame(const sd_script *script, const sd_script_frame *frame);
int sd_script_is_last_frame_at(const sd_script *script, int ticks);
int sd_script_is_first_frame(const sd_script *script, const sd_script_frame *frame);
int sd_script_is_first_frame_at(const sd_script *script, int ticks);
int sd_script_isset(const sd_script_frame *frame, const char* tag);
int sd_script_get(const sd_script_frame *frame, const char* tag);

#ifdef __cplusplus
}
#endif

#endif // _SD_SCRIPT_H