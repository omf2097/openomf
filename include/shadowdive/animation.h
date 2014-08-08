/*! \file 
 * \brief Functions for dealing with generic af/bk animation data.
 * \license MIT
 */ 

#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#include <stdint.h>
#include "shadowdive/sprite.h"
#include "shadowdive/colcoord.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_ANIMATION_STRING_MAX 1024
#define SD_EXTRA_STRING_MAX 512

#define SD_SPRITE_COUNT_MAX 255
#define SD_COLCOORD_COUNT_MAX 256
#define SD_EXTRASTR_COUNT_MAX 10

typedef struct {
    // Header
    int16_t start_x; ///< Animation start position, X-axis
    int16_t start_y; ///< Animation start position, Y-axis
    int32_t null; ///< Probably filler data
    uint16_t coord_count; ///< Number of collision coordinates in animation frames
    uint8_t sprite_count; ///< Number of sprites in animation
    uint8_t extra_string_count; ///< Number of extra strings in animation

    // Sprites and their collision coordinates
    sd_coord coord_table[SD_COLCOORD_COUNT_MAX]; ///< Collision coordinates
    sd_sprite *sprites[SD_SPRITE_COUNT_MAX]; ///< Sprites

    // String header & Extra strings
    char anim_string[SD_ANIMATION_STRING_MAX]; ///< Animation string
    char extra_strings[SD_EXTRASTR_COUNT_MAX][SD_EXTRA_STRING_MAX]; ///< Extra strings
} sd_animation;

/*! \brief Initialize animation structure
 *
 * Initializes the animation structure with empty values.
 *
 * \retval SD_INVALID_INPUT BK struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param animation Allocated animation struct pointer.
 */
int sd_animation_create(sd_animation* animation);

/*! \brief Copy animation structure
 *
 * Copies the contents of an animation structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_animation_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * animation structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success. 
 *
 * \param dst Destination animation struct pointer.
 * \param src Source animation struct pointer.
 */
int sd_animation_copy(sd_animation *dst, const sd_animation *src);

/*! \brief Free animation structure
 * 
 * Frees up all memory reserved by the animation structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param animation Animation struct to modify.
 */
void sd_animation_free(sd_animation *animation);

int sd_animation_get_coord_count(sd_animation *animation);
int sd_animation_set_coord(sd_animation *animation, int num, const sd_coord coord);
int sd_animation_push_coord(sd_animation *animation, const sd_coord coord);
int sd_animation_pop_coord(sd_animation *animation);
sd_coord* sd_animation_get_coord(sd_animation *animation, int num);

int sd_animation_set_anim_string(sd_animation *animation, const char *str);

int sd_animation_get_extra_string_count(sd_animation *animation);
int sd_animation_set_extra_string(sd_animation *animation, int num, const char *str);
int sd_animation_push_extra_string(sd_animation *anim, const char *str);
int sd_animation_pop_extra_string(sd_animation *anim);
char* sd_animation_get_extra_string(sd_animation *animation, int num);

int sd_animation_get_sprite_count(sd_animation *animation);
int sd_animation_set_sprite(sd_animation *animation, int num, const sd_sprite *sprite);
int sd_animation_push_sprite(sd_animation *animation, const sd_sprite *sprite);
int sd_animation_pop_sprite(sd_animation *animation);
sd_sprite* sd_animation_get_sprite(sd_animation *animation, int num);

#ifdef SD_USE_INTERNAL
int sd_animation_load(sd_reader *reader, sd_animation *animation);
int sd_animation_save(sd_writer *writer, const sd_animation *animation);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_ANIMATION_H
