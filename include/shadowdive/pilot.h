/*! \file 
 * \brief Pilot data handling
 * \license MIT
 */ 

#ifndef _SD_PILOT_H
#define _SD_PILOT_H

#include <stdint.h>
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    uint32_t unknown_a;
    char name[18];        ///< Pilot name
    uint16_t wins;        ///< Matches won by this pilot
    uint16_t losses;      ///< Matches lost by this pilot
    uint16_t robot_id;    ///< Har Identifier
    char stats[8];
    uint16_t offense;     ///< Offense value
    uint16_t defense;     ///< Defense value
    uint32_t money;       ///< Amount of money the pilot currently has
    uint8_t color_1;      ///< Color 1 field for the HAR
    uint8_t color_2;      ///< Color 2 field for the HAR
    uint8_t color_3;      ///< Color 3 field for the HAR
    char unk_block_a[107];
    uint16_t force_arena; ///< Tells if this pilot needs to play on a certain arena
    char unk_block_b[3];
    uint8_t movement;
    char unk_block_c[6];
    char enhancements[11];
    uint8_t unk_flag_a;
    uint8_t flags;
    uint8_t unk_flag_b;
    uint16_t reqs[5];
    uint16_t attitude[3];
    char unk_block_d[6];
    uint16_t ap_throw;
    uint16_t ap_special;
    uint16_t ap_jump;
    uint16_t ap_high;
    uint16_t ap_low;
    uint16_t ap_middle;
    uint16_t pref_jump;
    uint16_t pref_fwd;
    uint16_t pref_back;
    char unk_block_e[4];
    float learning;
    float forget;
    char unk_block_f[24];
    uint32_t winnings;
    char unk_block_g[166];
    uint16_t photo_id;  ///< Which face photo this pilot uses
} sd_pilot;

/*! \brief Initialize pilot struct
 *
 * Initializes the pilot structure with empty values.
 *
 * \retval SD_INVALID_INPUT Pilot struct pointer was NULL
 * \retval SD_SUCCESS Success.
 *
 * \param pilot Allocated pilot struct pointer.
 */
int sd_pilot_create(sd_pilot *pilot);

/*! \brief Free pilot structure
 *
 * Frees up all memory reserved by the pilot structure.
 * All contents will be freed, all pointers to contents will be invalid.
 *
 * \param pilot Pilot struct pointer.
 */
void sd_pilot_free(sd_pilot *pilot);

#ifdef SD_USE_INTERNAL
int sd_pilot_load(sd_reader *reader, sd_pilot *pilot);
int sd_pilot_save(sd_writer *writer, const sd_pilot *pilot);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_PILOT_H