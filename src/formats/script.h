/**
 * @file script.h
 * @brief Animation string parser
 * @details Functions and structs for reading, writing and modifying OMF:2097 animation strings.
 * @copyright MIT License
 * @date 2013-2026
 * @author Andrew Thompson
 * @author Tuomas Virtanen
 */

#ifndef SD_SCRIPT_H
#define SD_SCRIPT_H

#include "formats/script_tag.h"
#include "utils/str.h"
#include "utils/vector.h"
#include <stdint.h>

/** @brief Animation tag
 *
 * Describes a single tag in animation frame. Tags not in the known tag list are kept as TAG_INVALID
 * so that strings can be round-tripped, with their original character stored in the value field.
 */
typedef struct script_frame_tag {
    uint8_t key;       ///< Tag identifier (see script_tag)
    uint8_t has_param; ///< Tells if the tag has a parameter.
    int16_t value;     ///< Tag parameter value if has_param is set.
} script_frame_tag;

/** @brief Animation frame
 *
 * Describes a single frame in animation string.
 */
typedef struct script_frame {
    int sprite;   ///< Sprite ID that the frame relates to
    int tick_len; ///< Length of the frame in ticks
    vector tags;  ///< A list of tags in this frame
} script_frame;

/** @brief Animation script
 *
 * A single animation string. Contains multiple frames, which then contain tags.
 * A valid string must contain at least a single frame.
 */
typedef struct script {
    vector frames; ///< List of frames in this string
} script;

/** @brief Initialize script parser
 *
 * Initializes the script parser with empty values.
 *
 * @param script Allocated script struct pointer.
 */
void script_create(script *script);

/** @brief Clone a script
 *
 * @retval SD_SUCCESS Success.
 *
 * @param src Source script.
 * @param dst Destination script.
 */
int script_clone(const script *src, script *dst);

/** @brief Free script parser
 *
 * Frees up all memory reserved by the script parser structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * @param script Script struct to free.
 */
void script_free(script *script);

/** @brief Decode animation string
 *
 * Decodes an animation string to frames and tags. There must be at least
 * a single full frame, which means a frame number and length in ticks.
 *
 * @retval SD_ANIM_INVALID_STRING String is invalid; eg. doesn't have enough complete frames.
 * @retval SD_INVALID_TAG There was an invalid tag in string. invalid_pos will contain problematic position.
 * @retval SD_SUCCESS Decoding was successful.
 *
 * @param script Script structure to fill. Must be formatted using script_create().
 * @param str Animation string to parse
 * @param invalid_pos Will contain problematic position in string if SD_INVALID_TAG is returned. Will be ignored if set
 * to NULL.
 */
int script_decode(script *script, const char *str, int *invalid_pos);

/** @brief Decode animation string from a str object
 *
 * Like script_decode(), but takes a str object directly.
 *
 * @retval SD_ANIM_INVALID_STRING String is invalid; eg. doesn't have enough complete frames.
 * @retval SD_INVALID_TAG There was an invalid tag in string. invalid_pos will contain problematic position.
 * @retval SD_SUCCESS Decoding was successful.
 *
 * @param script Script structure to fill. Must be formatted using script_create().
 * @param src Animation string to parse
 * @param invalid_pos Will contain problematic position in string if SD_INVALID_TAG is returned. Will be ignored if set
 * to NULL.
 */
int script_decode_str(script *script, const str *src, int *invalid_pos);

/** @brief Encode animation string
 *
 * Encodes the animation script structure back to an animation string.
 * Note that if the decoded string contained oddities like prefixed zeroes in
 * front of numbers, those will be automatically fixed. Therefore the output
 * string may not be exactly like the input string.
 *
 * @retval SD_SUCCESS Successful operation
 *
 * @param script Script structure to encode
 * @param dst Target string object. Must be initialized!
 */
int script_encode(const script *script, str *dst);

/** @brief Encode animation frame string
 *
 * Encodes the animation frame structure to an animation string.
 *
 * @retval SD_SUCCESS Successful operation
 *
 * @param frame Script frame to encode
 * @param dst Target string object. Must be initialized!
 */
int script_encode_frame(const script_frame *frame, str *dst);

/** @brief Find the total duration of the script
 *
 * Finds the total duration of the script in game ticks. Essentially
 * just adds up all the frame lengths.
 *
 * @param script Script structure to check
 * @return The total duration of the script in game ticks
 */
unsigned script_get_total_ticks(const script *script);

/** @brief Find the tick position at the start of the given frame.
 *
 * @param script Script structure to check
 * @param frame_id Frame ID to search
 * @return Tick position at the frame
 */
int script_get_tick_pos_at_frame(const script *script, int frame_id);

/** @brief Find the tick length of the given frame
 *
 * @param script Script structure to check
 * @param frame_id Frame ID to search
 * @return Tick length of the frame, 0 if frame_id does not exist.
 */
int script_get_tick_len_at_frame(const script *script, int frame_id);

/** @brief Find the sprite ID of the given frame
 *
 * @param script Script structure to check
 * @param frame_id Frame ID to search
 * @return Sprite ID of the given frame, 0 if frame_id does not exist.
 */
int script_get_sprite_at_frame(const script *script, int frame_id);

/** @brief Returns the frame at a given tick
 *
 * Finds the frame at the given moment in time (tick). If the ticks value is either
 * larger than the total duration of the script or a negative value, the function will return NULL.
 * If script variable is NULL, the function will also return NULL. Otherwise the function will
 * return the frame at the moment of the given tick.
 *
 * The tick values will start at 0, so if the tick length of a frame is 140, the last tick will be 139.
 *
 * The returned frame is owned by the library; do not attempt to free it.
 *
 * @param script The script structure to read
 * @param ticks A position in time in game ticks
 * @return Frame pointer or NULL
 */
const script_frame *script_get_frame_at(const script *script, int ticks);

/** @brief Returns the frame at a given position
 *
 * Returns the frame at a given position. If the frame number given is less than 0
 * or larger or equal than the number of frames, NULL will be returned. A NULL value
 * for the script pointer will also cause a NULL to be returned. Otherwise the function
 * will return the frame at the given position.
 *
 * The returned frame is owned by the library; do not attempt to free it.
 *
 * @param script The script structure to read
 * @param frame_number Frame number to get
 * @return Frame pointer or NULL
 */
const script_frame *script_get_frame(const script *script, int frame_number);

/** @brief Returns the number of frames in the script
 *
 * @param script The script structure to read
 * @return Number of frames, or 0 if script is NULL
 */
int script_get_frame_count(const script *script);

/** @brief Returns the information of a tag in a frame.
 *
 * Returns the tag descriptor for the instance of a tag in frame. A NULL for either
 * parameter will result in a NULL being returned. If the requested tag does not
 * exist in the frame, a NULL will be also returned.
 *
 * The returned tag information struct is owned by the library; do not attempt to free it.
 *
 * @param frame Frame structure to read
 * @param tag Tag to look for. Must have a trailing zero.
 * @return Tag descriptor pointer or NULL
 */
const script_frame_tag *script_get_tag_by_name(const script_frame *frame, const char *tag);

/** @brief Returns the information of a tag in a frame, by tag id.
 *
 * @param frame Frame structure to read
 * @param id Tag id to look for
 * @return Tag descriptor pointer or NULL
 */
const script_frame_tag *script_get_tag_by_id(const script_frame *frame, script_tag id);

/** @brief Tells if the frame changed between two points in time
 *
 * Tells if the frame changed between two game ticks. If the tick times are equal,
 * if frame struct pointer is null, or if the frame didn't change, 0 will be returned.
 * If the frame changed, 1 will be returned.
 *
 * Note that a change between zero and negative value will also count as a frame change.
 * Eg. switching between "animation not started" and first frame wil lcause 1.
 * Also, change from a tick in animation range to a tick outside of animation will cause
 * 1 to be returned. However, a change from "not started" (-1) to "finished" (tick > total_ticks)
 * will cause 0 to be returned.
 *
 * @param script The script structure to inspect
 * @param tick_start Position 1
 * @param tick_stop Position 2
 * @return 1 or 0, whether the frame changed or not.
 */
int script_frame_changed(const script *script, int tick_start, int tick_stop);

/** @brief Returns the array index of frame
 *
 * This simply returns the array index of the given frame. Comparison to the frames
 * in the script will be done by pointer. If the frame does not exist in the animation,
 * -1 will be returned.
 *
 * @param script The script structure to inspect
 * @param frame Frame structure to find the index of
 * @return Index of the frame
 */
int script_get_frame_index(const script *script, const script_frame *frame);

/** @brief Returns the array index of frame at given tick position
 *
 * Returns the array index of the frame at a given tick position. If the tick is valid
 * and in the range of the animation, an index will be returned. If the tick is outside
 * the animation range, -1 will be returned.
 *
 * @param script The script structure to inspect
 * @param ticks Tick position to find
 * @return Index of the frame
 */
int script_get_frame_index_at(const script *script, unsigned ticks);

/** @brief Tells if the frame is the last frame in animation.
 *
 * Tells if the given frame pointer is the last frame in the given animation.
 * If script or frame pointer is null, or frame is not the last frame, 0 will be returned.
 * Otherwise 1 will be returned.
 *
 * @param script The script structure to inspect
 * @param frame Frame structure to find
 * @return 1 or 0
 */
int script_is_last_frame(const script *script, const script_frame *frame);

/** @brief Tells if the frame index is the last frame in animation.
 *
 * Tells if the given frame index is the last frame in the given animation.
 * If script pointer is null, or frame is not the last frame, 0 will be returned.
 * Otherwise 1 will be returned.
 *
 * @param script The script structure to inspect
 * @param ticks Tick position to find
 * @return 1 or 0
 */
int script_is_last_frame_at(const script *script, int ticks);

/** @brief Tells if the frame is the first frame in animation.
 *
 * Tells if the given frame pointer is the first frame in the given animation.
 * If script or frame pointer is null, or frame is not the first frame, 0 will be returned.
 * Otherwise 1 will be returned.
 *
 * @param script The script structure to inspect
 * @param frame Frame structure to find
 * @return 1 or 0
 */
int script_is_first_frame(const script *script, const script_frame *frame);

/** @brief Tells if the frame index is the first frame in animation.
 *
 * Tells if the given frame index is the first frame in the given animation.
 * If script pointer is null, or frame is not the first frame, 0 will be returned.
 * Otherwise 1 will be returned.
 *
 * @param script The script structure to inspect
 * @param ticks Tick position to find
 * @return 1 or 0
 */
int script_is_first_frame_at(const script *script, int ticks);

/** @brief Tells if the tag is set in frame
 *
 * Tells if the given tag is set in the given frame, Returns 1 if tag is set,
 * otherwise 0.
 *
 * @param frame The frame structure to inspect
 * @param tag Tag name to find
 * @return 1 or 0
 */
int script_is_tag_set_by_name(const script_frame *frame, const char *tag);

/** @brief Tells if the tag id is set in frame
 *
 * Tells if the tag, identified by its enum value, is set in the frame.
 *
 * @param frame The frame structure to inspect
 * @param id Tag id to find
 * @return 1 or 0
 */
int script_is_tag_set_by_id(const script_frame *frame, script_tag id);

/** @brief Returns the tag value in frame
 *
 * Returns the parameter value of a tag in a given frame. Note that if the tag doesn't
 * exist in frame, or tag doesn't have a parameter value, 0 will be returned. Otherwise
 * the tag parameter value will be returned.
 *
 * @param frame The frame structure to inspect
 * @param tag Tag name to find
 * @return Tag parameter value or 0.
 */
int script_get_tag_value_by_name(const script_frame *frame, const char *tag);

/** @brief Returns the tag value in frame, by tag id
 *
 * @param frame The frame structure to inspect
 * @param id Tag id to find
 * @return Tag parameter value or 0.
 */
int script_get_tag_value_by_id(const script_frame *frame, script_tag id);

/** @brief Returns the next frame number with a given sprite ID
 *
 * Returns the next frame number with the given sprite number. Sprite numbers start from 0 and go to
 * the amount of sprites in the frame.
 *
 * If script is NULL, sprite_id is smaller than 0 or larger than the amount of sprites in the
 * script, or current_tick is larger than the length of the script animation, or sprite frame
 * with the sprite_id does not exist in script, -1 value will be returned. Otherwise
 * the frame number will be returned.
 *
 * The search will start from the next frame from the one that current_tick is pointing at.
 *
 * @param script Script structure to search through
 * @param sprite_id Sprite ID to search for
 * @param current_tick Current tick time
 * @return Frame ID or -1 on error
 */
int script_get_next_frame_with_sprite(const script *script, int sprite_id, unsigned current_tick);

/** @brief Returns the next frame number with a given tag
 *
 * Returns the next frame number with the given tag.
 *
 * If script or tag is NULL, current_tick is larger than the length of the script animation,
 * or sprite frame with the tag does not exist in script, -1 value will be returned. Otherwise
 * the frame number will be returned.
 *
 * The search will start from the next frame from the one that current_tick is pointing at.
 *
 * @param script Script structure to search through
 * @param tag Tag to search for
 * @param current_tick Current tick time
 * @return Frame ID or -1 on error
 */
int script_get_next_frame_with_tag(const script *script, const char *tag, uint32_t current_tick);

/** @brief Returns the next frame number with a given tag id
 *
 * @param script Script structure to search through
 * @param id Tag id to search for
 * @param current_tick Current tick time
 * @return Frame ID or -1 on error
 */
int script_get_next_frame_with_tag_id(const script *script, script_tag id, uint32_t current_tick);

/** @brief Sets a tag for the given frame
 *
 * Sets the tag for the given frame. If the tag has not been set previously, a new tag
 * entry will be created. If the tag has been set previously, and the tag accepts value
 * parameters, a new parameter will be set for the tag. Otherwise this function does nothing.
 *
 * @retval SD_SUCCESS On succesful tag set operation
 * @retval SD_INVALID_INPUT if the tag is invalid or the frame does not exist.
 *
 * @param script Script to modify
 * @param frame_id Frame ID to modify
 * @param tag Tag to add
 * @param value Value to set, if tag allows values.
 */
int script_set_tag(script *script, int frame_id, const char *tag, int value);

/** @brief Deletes a tag from the given frame
 *
 * Deletes the tag from the given frame. If the tag does not exist, does nothing.
 *
 * @retval SD_SUCCESS On succesful tag set operation
 * @retval SD_INVALID_INPUT if the frame does not exist.
 *
 * @param script Script to modify
 * @param frame_id Frame ID to modify
 * @param tag Tag to delete
 */
int script_delete_tag(script *script, int frame_id, const char *tag);

/** @brief Append a new frame to the end of the frame list
 *
 * Appends a new frame to the end of the frame list, and sets its tick length and sprite ID.
 * Tag list will be empty after creation.
 *
 * @retval SD_SUCCESS on successful frame creation
 *
 * @param script Script to modify
 * @param tick_len Tick length of the new frame
 * @param sprite_id Sprite ID for the new frame
 */
int script_append_frame(script *script, int tick_len, int sprite_id);

/** @brief Clear frame tags
 *
 * Clears all tags from the given frame.
 *
 * @retval SD_SUCCESS on successful removal
 * @retval SD_INVALID_INPUT if frame_id is nonexistent
 *
 * @param script Script to modify
 * @param frame_id Valid frame ID with tags to clear
 */
int script_clear_tags(script *script, int frame_id);

/** @brief Set frame tick length
 *
 * Sets a new tick length for the given frame.
 *
 * @retval SD_SUCCESS on successful operation
 * @retval SD_INVALID_INPUT if frame_id is nonexistent
 *
 * @param script Script to modify
 * @param frame_id Frame ID to set tick length to
 * @param duration Tick length to set
 */
int script_set_tick_len_at_frame(script *script, int frame_id, int duration);

/** @brief Set frame sprite ID
 *
 * Sets the sprite ID of the given frame.
 *
 * @retval SD_SUCCESS on successful operation
 * @retval SD_INVALID_INPUT if frame_id is invalid or sprite_id is invalid.
 *
 * @param script Script to modify
 * @param frame_id Frame ID to set sprite ID to
 * @param sprite_id Sprite ID to set
 */
int script_set_sprite_at_frame(script *script, int frame_id, int sprite_id);

/** @brief Returns the frame ID by frame letter.
 *
 * @param letter Frame letter ('A', 'B', etc.)
 * @return Frame ID ('A' => 0, ...)
 */
int script_letter_to_frame(char letter);

/** @brief Returns the frame letter by frame ID.
 *
 * @param frame_id Frame ID (0 ... n)
 * @return Frame letter (0 => 'A', ...)
 */
char script_frame_to_letter(int frame_id);

/** Initializes a script frame.
 *
 * @param frame Frame struct to initialize
 * @param tick_len Frame length in ticks
 * @param sprite Sprite ID to use
 */
void script_frame_create(script_frame *frame, int tick_len, int sprite);

/** Frees a script frame
 *
 * @param frame Frame to free
 */
void script_frame_free(script_frame *frame);

/** Add a tag to a frame.
 *
 * @param frame Frame to add into
 * @param key Key name, e.h. "bpp" or "m"
 * @param value Value. This is only used if tag supports it.
 * @return True if tag was valid, false if not.
 */
bool script_frame_add_tag(script_frame *frame, const char *key, int value);

#endif // SD_SCRIPT_H
