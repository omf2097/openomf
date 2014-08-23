#include <stdlib.h>
#include <string.h>

#include "shadowdive/error.h"
#include "shadowdive/pilot.h"

#define PILOT_BLOCK_LENGTH 428

int sd_pilot_create(sd_pilot *pilot) {
    if(pilot == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(pilot, 0, sizeof(sd_pilot));
    return SD_SUCCESS;
}

void sd_pilot_free(sd_pilot *pilot) {}

// Reads exactly 24 + 8 + 11 = 43 bytes
void sd_pilot_load_player_from_mem(sd_mreader *mr, sd_pilot *pilot) {
    sd_mread_buf(mr, pilot->name, 18);
    pilot->wins =        sd_mread_uword(mr);
    pilot->losses =      sd_mread_uword(mr);
    pilot->rank =        sd_mread_ubyte(mr);
    pilot->har_id =      sd_mread_ubyte(mr);

    uint16_t stats_a =   sd_mread_uword(mr);
    uint16_t stats_b =   sd_mread_uword(mr);
    uint16_t stats_c =   sd_mread_uword(mr);
    uint8_t stats_d =    sd_mread_ubyte(mr);
    pilot->arm_power = (stats_a >> 0) & 0x1F;
    pilot->leg_power = (stats_a >> 5) & 0x1F;
    pilot->arm_speed = (stats_a >> 10) & 0x1F;
    pilot->leg_speed = (stats_b >> 0) & 0x1F;
    pilot->armor     = (stats_b >> 5) & 0x1F;
    pilot->stun_resistance = (stats_b >> 10) & 0x1F;
    pilot->power = (stats_c >> 0) & 0x7F;
    pilot->agility = (stats_c >> 7) & 0x7F;
    pilot->endurance = (stats_d >> 0) & 0x7F;
    pilot->unknown_stat = sd_mread_ubyte(mr);

    pilot->offense =     sd_mread_uword(mr);
    pilot->defense =     sd_mread_uword(mr);
    pilot->money =       sd_mread_udword(mr);
    pilot->color_1 =     sd_mread_ubyte(mr);
    pilot->color_2 =     sd_mread_ubyte(mr);
    pilot->color_3 =     sd_mread_ubyte(mr);
}

void sd_pilot_load_from_mem(sd_mreader *mr, sd_pilot *pilot) {
    pilot->unknown_a =   sd_mread_udword(mr);

    sd_pilot_load_player_from_mem(mr, pilot);

    sd_mread_buf(mr, pilot->trn_name, 13);
    sd_mread_buf(mr, pilot->trn_desc, 31);
    sd_mread_buf(mr, pilot->trn_image, 13);

    sd_mread_buf(mr, pilot->unk_block_a, 48);
    pilot->pilot_id =    sd_mread_ubyte(mr);
    pilot->unknown_k =   sd_mread_ubyte(mr);
    pilot->force_arena = sd_mread_uword(mr);
    pilot->difficulty = (sd_mread_ubyte(mr) >> 3) & 0x3; // 155-156
    sd_mread_buf(mr, pilot->unk_block_b, 2);
    pilot->movement =    sd_mread_ubyte(mr);
    sd_mread_buf(mr, pilot->unk_block_c, 6);
    sd_mread_buf(mr, pilot->enhancements, 11);

    pilot->unk_flag_a =  sd_mread_ubyte(mr);
    pilot->flags =       sd_mread_ubyte(mr);
    pilot->unk_flag_b =  sd_mread_ubyte(mr);
    sd_mread_buf(mr, (char*)pilot->reqs, 10);
    sd_mread_buf(mr, (char*)pilot->attitude, 6);
    sd_mread_buf(mr, pilot->unk_block_d, 6);

    pilot->ap_throw =    sd_mread_word(mr);
    pilot->ap_special =  sd_mread_word(mr);
    pilot->ap_jump =     sd_mread_word(mr);
    pilot->ap_high =     sd_mread_word(mr);
    pilot->ap_low =      sd_mread_word(mr);
    pilot->ap_middle =   sd_mread_word(mr);
    pilot->pref_jump =   sd_mread_word(mr);
    pilot->pref_fwd =    sd_mread_word(mr);
    pilot->pref_back =   sd_mread_word(mr);

    pilot->unknown_e =   sd_mread_udword(mr);
    pilot->learning =    sd_mread_float(mr);
    pilot->forget =      sd_mread_float(mr);
    sd_mread_buf(mr, pilot->unk_block_f, 14);
    pilot->enemies_inc_unranked = sd_mread_uword(mr);
    pilot->enemies_ex_unranked = sd_mread_uword(mr);
    sd_mread_buf(mr, pilot->unk_block_g, 6);
    pilot->winnings =    sd_mread_udword(mr);
    sd_mread_buf(mr, pilot->unk_block_h, 166);
    pilot->photo_id =    sd_mread_uword(mr);
}

int sd_pilot_load(sd_reader *reader, sd_pilot *pilot) {
    if(reader == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    // Read block, XOR, Read to pilot, free memory
    sd_mreader *mr = sd_mreader_open_from_reader(reader, PILOT_BLOCK_LENGTH);
    sd_mreader_xor(mr, PILOT_BLOCK_LENGTH & 0xFF);
    sd_pilot_load_from_mem(mr, pilot);
    sd_mreader_close(mr);
    return SD_SUCCESS;
}

void sd_pilot_save_player_to_mem(sd_mwriter *w, const sd_pilot *pilot) {
    sd_mwrite_buf(w, pilot->name, 18);
    sd_mwrite_uword(w, pilot->wins);
    sd_mwrite_uword(w, pilot->losses);
    sd_mwrite_ubyte(w, pilot->rank);
    sd_mwrite_ubyte(w, pilot->har_id);

    uint16_t stats_a = 0, stats_b = 0, stats_c = 0;
    uint8_t stats_d = 0;
    stats_a |= (pilot->arm_power & 0x1F) << 0;
    stats_a |= (pilot->leg_power & 0x1F) << 5;
    stats_a |= (pilot->arm_speed & 0x1F) << 10;
    stats_b |= (pilot->leg_speed & 0x1F) << 0;
    stats_b |= (pilot->armor & 0x1F) << 5;
    stats_b |= (pilot->stun_resistance & 0x1F) << 10;
    stats_c |= (pilot->power & 0x7F) << 0;
    stats_c |= (pilot->agility & 0x7F) << 7;
    stats_d |= (pilot->endurance & 0x7F) << 0;
    sd_mwrite_uword(w, stats_a);
    sd_mwrite_uword(w, stats_b);
    sd_mwrite_uword(w, stats_c);
    sd_mwrite_ubyte(w, stats_d);
    sd_mwrite_ubyte(w, pilot->unknown_stat);

    sd_mwrite_uword(w, pilot->offense);
    sd_mwrite_uword(w, pilot->defense);
    sd_mwrite_udword(w, pilot->money);
    sd_mwrite_ubyte(w, pilot->color_1);
    sd_mwrite_ubyte(w, pilot->color_2);
    sd_mwrite_ubyte(w, pilot->color_3);
}

void sd_pilot_save_to_mem(sd_mwriter *w, const sd_pilot *pilot) {
    // Write the pilot block
    sd_mwrite_udword(w, pilot->unknown_a);

    sd_pilot_save_player_to_mem(w, pilot);

    sd_mwrite_buf(w, pilot->trn_name, 13);
    sd_mwrite_buf(w, pilot->trn_desc, 31);
    sd_mwrite_buf(w, pilot->trn_image, 13);

    sd_mwrite_buf(w, pilot->unk_block_a, 48);
    sd_mwrite_ubyte(w, pilot->pilot_id);
    sd_mwrite_ubyte(w, pilot->unknown_k);
    sd_mwrite_uword(w, pilot->force_arena);
    sd_mwrite_ubyte(w, (pilot->difficulty & 0x3) << 3);
    sd_mwrite_buf(w, pilot->unk_block_b, 2);
    sd_mwrite_ubyte(w, pilot->movement);
    sd_mwrite_buf(w, pilot->unk_block_c, 6);
    sd_mwrite_buf(w, pilot->enhancements, 11);

    sd_mwrite_ubyte(w, pilot->unk_flag_a);
    sd_mwrite_ubyte(w, pilot->flags);
    sd_mwrite_ubyte(w, pilot->unk_flag_b);
    sd_mwrite_buf(w, (char*)pilot->reqs, 10);
    sd_mwrite_buf(w, (char*)pilot->attitude, 6);
    sd_mwrite_buf(w, pilot->unk_block_d, 6);

    sd_mwrite_uword(w, pilot->ap_throw);
    sd_mwrite_uword(w, pilot->ap_special);
    sd_mwrite_uword(w, pilot->ap_jump);
    sd_mwrite_uword(w, pilot->ap_high);
    sd_mwrite_uword(w, pilot->ap_low);
    sd_mwrite_uword(w, pilot->ap_middle);
    sd_mwrite_uword(w, pilot->pref_jump);
    sd_mwrite_uword(w, pilot->pref_fwd);
    sd_mwrite_uword(w, pilot->pref_back);

    sd_mwrite_udword(w, pilot->unknown_e);
    sd_mwrite_float(w, pilot->learning);
    sd_mwrite_float(w, pilot->forget);
    sd_mwrite_buf(w, pilot->unk_block_f, 14);
    sd_mwrite_uword(w, pilot->enemies_inc_unranked);
    sd_mwrite_uword(w, pilot->enemies_ex_unranked);
    sd_mwrite_buf(w, pilot->unk_block_g, 6);
    sd_mwrite_udword(w, pilot->winnings);
    sd_mwrite_buf(w, pilot->unk_block_h, 166);
    sd_mwrite_uword(w, pilot->photo_id);
}

int sd_pilot_save(sd_writer *fw, const sd_pilot *pilot) {
    if(fw == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    // Copy, XOR, save and close
    sd_mwriter *w = sd_mwriter_open();
    sd_pilot_save_to_mem(w, pilot);
    sd_mwriter_xor(w, PILOT_BLOCK_LENGTH & 0xFF);
    sd_mwriter_save(w, fw);
    sd_mwriter_close(w);
    return SD_SUCCESS;
}