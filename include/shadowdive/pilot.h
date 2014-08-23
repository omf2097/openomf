/*! \file 
 * \brief Pilot data handling
 * \license MIT
 */ 

#ifndef _SD_PILOT_H
#define _SD_PILOT_H

#include <stdint.h>
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/memreader.h"
    #include "shadowdive/internal/writer.h"
    #include "shadowdive/internal/memwriter.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    uint32_t unknown_a;
    char name[18];           ///< Pilot name
    uint16_t wins;           ///< Matches won by this pilot
    uint16_t losses;         ///< Matches lost by this pilot
    uint8_t rank;            ///< Rank
    uint8_t har_id;          ///< Har Identifier
    uint8_t arm_power;       ///< HAR Arm power
    uint8_t leg_power;       ///< HAR Leg power
    uint8_t arm_speed;       ///< HAR Arm speed
    uint8_t leg_speed;       ///< HAR Leg speed
    uint8_t armor;           ///< Har armor
    uint8_t stun_resistance; ///< Har stun resistance
    uint8_t power;           ///< Har power
    uint8_t agility;         ///< Har agility
    uint8_t endurance;       ///< Har endurance
    uint8_t unknown_stat;
    uint16_t offense;        ///< Offense value
    uint16_t defense;        ///< Defense value
    uint32_t money;          ///< Amount of money the pilot currently has
    uint8_t color_1;         ///< Color 1 field for the HAR
    uint8_t color_2;         ///< Color 2 field for the HAR
    uint8_t color_3;         ///< Color 3 field for the HAR
    char trn_name[13];       ///< Tournament file
    char trn_desc[31];       ///< Tournament description
    char trn_image[13];      ///< Tournament image file
    char unk_block_a[48];
    uint8_t pilot_id;        ///< Pilot ID
    uint8_t unknown_k;
    uint16_t force_arena;    ///< Tells if this pilot needs to play on a certain arena
    uint8_t difficulty;      ///< Difficulty setting
    char unk_block_b[2];
    uint8_t movement;
    char unk_block_c[6];
    char enhancements[11];
    uint8_t unk_flag_a;
    uint8_t flags;
    uint8_t unk_flag_b;
    uint16_t reqs[5];
    uint16_t attitude[3];
    char unk_block_d[6];
    int16_t ap_throw;
    int16_t ap_special;
    int16_t ap_jump;
    int16_t ap_high;
    int16_t ap_low;
    int16_t ap_middle;
    int16_t pref_jump;
    int16_t pref_fwd;
    int16_t pref_back;
    uint32_t unknown_e;
    float learning;
    float forget;
    char unk_block_f[24];
    uint32_t winnings;
    char unk_block_g[6];
    uint16_t enemies_inc_unranked;
    uint16_t enemies_ex_unranked;
    char unk_block_h[166];
    uint16_t photo_id;        ///< Which face photo this pilot uses
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
void sd_pilot_load_player_from_mem(sd_mreader *mreader, sd_pilot *pilot);
void sd_pilot_load_from_mem(sd_mreader *mreader, sd_pilot *pilot);
int sd_pilot_load(sd_reader *reader, sd_pilot *pilot);
void sd_pilot_save_player_to_mem(sd_mwriter *mwriter, const sd_pilot *pilot);
void sd_pilot_save_to_mem(sd_mwriter *mwriter, const sd_pilot *pilot);
int sd_pilot_save(sd_writer *writer, const sd_pilot *pilot);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_PILOT_H