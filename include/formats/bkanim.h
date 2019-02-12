/*! \file
 * \brief BK animation data handling.
 * \details Functions and structs for reading, writing and modifying OMF:2097 BK specific animation structures.
 * \copyright MIT license.
 * \date 2013-2014
 * \author Andrew Thompson
 * \author Tuomas Virtanen
 */

#ifndef _SD_BK_ANIMS
#define _SD_BK_ANIMS

#include <stdint.h>
#include "formats/animation.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"


#define SD_BK_FOOTER_STRING_MAX 512 ///< Max BK footer string length

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief BK specific animation information
 *
 * Information about the BK specific animation things.
 */
typedef struct {
    uint8_t null; ///< Always 0 ?
    uint8_t chain_hit; ///< Animation to chain to if collision/hit
    uint8_t chain_no_hit; ///< Animation to chain to on no collision/hit
    uint8_t load_on_start; ///< Actually repeat flag
    uint16_t probability; ///< Probability of animation
    uint8_t hazard_damage; ///< Hazard damage on hit
    char footer_string[SD_BK_FOOTER_STRING_MAX]; ///< Footer string
    sd_animation *animation; ///< Animation ptr or NULL. On BK save, must be != NULL.
} sd_bk_anim;


/*! \brief Initialize BK animation info structure
 *
 * Initializes the BK animation info structure with empty values.
 *
 * \retval SD_INVALID_INPUT BK struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param bka Allocated BK animation info struct pointer.
 */
int sd_bk_anim_create(sd_bk_anim *bka);

/*! \brief Copy BK animation info structure
 *
 * Copies the contents of a BK animation info structure. _ALL_ internals will be copied.
 * The copied structure must be freed using sd_bk_anim_free().
 *
 * Destination buffer does not need to be cleared. Source buffer must be a valid
 * move structure, or problems are likely to appear.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Destination struct should now be considered invalid and freed.
 * \retval SD_INVALID_INPUT Either input value was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param dst Destination BK animation info struct pointer.
 * \param src Source BK animation info struct pointer.
 */
int sd_bk_anim_copy(sd_bk_anim *dst, const sd_bk_anim *src);

/*! \brief Free BK animation info structure
 *
 * Frees up all memory reserved by the BK animation info structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param bka BK animation info struct to modify.
 */
void sd_bk_anim_free(sd_bk_anim *bka);

/*! \brief Set animation struct for BK animation info struct
 *
 * Sets an animation for the BK animation info struct. Animation will be copied,
 * so remember to free your local copy yourself. Note that any valid
 * BK anim info struct should ALWAYS contain an animation. Otherwise there will be problems
 * eg. saving the BK file.
 *
 * A NULL value for animation field will result in bka->animation field getting freed.
 *
 * \retval SD_OUT_OF_MEMORY Memory ran out. Animation field will be NULL.
 * \retval SD_INVALID_INPUT Move struct pointer was NULL.
 * \retval SD_SUCCESS Success.
 *
 * \param bka BK animation info struct to modify.
 * \param animation Animation to set. This will be copied.
 */
int sd_bk_anim_set_animation(sd_bk_anim *bka, const sd_animation *animation);

/*! \brief Get the current animation
 *
 * Returns a pointer to the current animation for the BK animation info struct.
 * If animation is not set, NULL will be returned.
 *
 * \retval NULL Animation does not exist
 * \retval sd_animation* Success.
 *
 * \param bka BK animation info struct to modify.
 */
sd_animation* sd_bk_anim_get_animation(const sd_bk_anim *bka);

/*! \brief Set BK animation info footer string
 *
 * Sets the BK animation info footer string for the struct. Maximum length is
 * 512 bytes. Longer strings will result in error.
 *
 * \retval SD_INVALID_INPUT Input string was too long.
 * \retval SD_SUCCESS Success.
 *
 * \param bka BK animation info struct to modify.
 * \param data String to set.
 */
int sd_bk_set_anim_string(sd_bk_anim *bka, const char *data);

int sd_bk_anim_load(sd_reader *reader, sd_bk_anim *bka);
int sd_bk_anim_save(sd_writer *writer, const sd_bk_anim *bka);

#ifdef __cplusplus
}
#endif

#endif // _SD_BK_ANIMS
