#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/pilot.h"
#include "utils/allocator.h"

#define PILOT_BLOCK_LENGTH 428

int sd_pilot_create(sd_pilot *pilot) {
    if(pilot == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(pilot, 0, sizeof(sd_pilot));
    return SD_SUCCESS;
}

void sd_pilot_clone(sd_pilot *dest, const sd_pilot *src) {
    memcpy(dest, src, sizeof(sd_pilot));
    for(int m = 0; m < 10; m++) {
        if(src->quotes[m] != NULL) {
            dest->quotes[m] = strdup(src->quotes[m]);
        }
    }
}

void sd_pilot_free(sd_pilot **pilot) {
    if(*pilot == NULL)
        return;
    for(int m = 0; m < 10; m++) {
        omf_free((*pilot)->quotes[m]);
    }
    omf_free(*pilot);
}

// Reads exactly 24 + 8 + 11 = 43 bytes
void sd_pilot_load_player_from_mem(memreader *mr, sd_pilot *pilot) {
    memread_buf(mr, pilot->name, 18);
    pilot->wins = memread_uword(mr);
    pilot->losses = memread_uword(mr);
    pilot->rank = memread_ubyte(mr);
    pilot->har_id = memread_ubyte(mr);

    uint16_t stats_a = memread_uword(mr);
    uint16_t stats_b = memread_uword(mr);
    uint16_t stats_c = memread_uword(mr);
    uint8_t stats_d = memread_ubyte(mr);
    pilot->arm_power = (stats_a >> 0) & 0x1F;
    pilot->leg_power = (stats_a >> 5) & 0x1F;
    pilot->arm_speed = (stats_a >> 10) & 0x1F;
    pilot->leg_speed = (stats_b >> 0) & 0x1F;
    pilot->armor = (stats_b >> 5) & 0x1F;
    pilot->stun_resistance = (stats_b >> 10) & 0x1F;
    pilot->agility = (stats_c >> 0) & 0x7F;
    pilot->power = (stats_c >> 7) & 0x7F;
    pilot->endurance = (stats_d >> 0) & 0x7F;
    sd_mskip(mr, 1);

    pilot->offense = memread_uword(mr);
    pilot->defense = memread_uword(mr);
    pilot->money = memread_dword(mr);
    sd_pilot_set_player_color(pilot, PRIMARY, memread_ubyte(mr));
    sd_pilot_set_player_color(pilot, SECONDARY, memread_ubyte(mr));
    sd_pilot_set_player_color(pilot, TERTIARY, memread_ubyte(mr));
}

void sd_pilot_load_from_mem(memreader *mr, sd_pilot *pilot) {
    pilot->unknown_a = memread_udword(mr);

    sd_pilot_load_player_from_mem(mr, pilot);

    memread_buf(mr, pilot->trn_name, 13);
    memread_buf(mr, pilot->trn_desc, 31);
    memread_buf(mr, pilot->trn_image, 13);

    pilot->unk_f_c = memread_float(mr);
    pilot->unk_f_d = memread_float(mr);
    sd_mskip(mr, 40); // Pointless pointers
    pilot->pilot_id = memread_ubyte(mr);
    pilot->unknown_k = memread_ubyte(mr);
    pilot->force_arena = memread_uword(mr);
    pilot->difficulty = (memread_ubyte(mr) >> 3) & 0x3; // 155-156
    memread_buf(mr, pilot->unk_block_b, 2);
    pilot->movement = memread_ubyte(mr);
    memread_buf(mr, (char *)pilot->unk_block_c, 6);
    memread_buf(mr, pilot->enhancements, 11);

    // Flags (3)
    sd_mskip(mr, 1);                       // Nothing here
    uint8_t req_flags = memread_ubyte(mr); // Secret, only fight once flags
    pilot->secret = (req_flags & 0x02) ? 1 : 0;
    pilot->only_fight_once = (req_flags & 0x08) ? 1 : 0;
    sd_mskip(mr, 1); // Nothing here either

    // Requirements (10)
    uint16_t reqs[5];
    memread_buf(mr, (char *)reqs, 10);
    pilot->req_rank = reqs[0] & 0xFF;
    pilot->req_max_rank = (reqs[0] >> 8) & 0xFF;
    pilot->req_fighter = reqs[1] & 0x1F;
    pilot->req_difficulty = (reqs[2] >> 8) & 0x0F;
    pilot->req_enemy = (reqs[2] & 0xFF);
    pilot->req_vitality = reqs[3] & 0x7F;
    pilot->req_accuracy = (reqs[3] >> 7) & 0x7F;
    pilot->req_avg_dmg = reqs[4] & 0x7F;
    pilot->req_scrap = (reqs[4] & 0x80) ? 1 : 0;
    pilot->req_destroy = ((reqs[4] >> 8) & 0x01) ? 1 : 0;

    // Attitude
    uint16_t att[3];
    memread_buf(mr, (char *)att, 6);
    pilot->att_normal = (att[0] >> 4) & 0x7F;
    pilot->att_hyper = att[1] & 0x7F;
    pilot->att_jump = (att[1] >> 7) & 0x7F;
    pilot->att_def = att[2] & 0x7F;
    pilot->att_sniper = (att[2] >> 7) & 0x7F;

    memread_buf(mr, (char *)pilot->unk_block_d, 6);

    pilot->ap_throw = memread_word(mr);
    pilot->ap_special = memread_word(mr);
    pilot->ap_jump = memread_word(mr);
    pilot->ap_high = memread_word(mr);
    pilot->ap_low = memread_word(mr);
    pilot->ap_middle = memread_word(mr);
    pilot->pref_jump = memread_word(mr);
    pilot->pref_fwd = memread_word(mr);
    pilot->pref_back = memread_word(mr);

    pilot->unknown_e = memread_udword(mr);
    pilot->learning = memread_float(mr);
    pilot->forget = memread_float(mr);
    memread_buf(mr, pilot->unk_block_f, 14);
    pilot->enemies_inc_unranked = memread_uword(mr);
    pilot->enemies_ex_unranked = memread_uword(mr);
    pilot->unk_d_a = memread_uword(mr);
    pilot->har_trades = memread_udword(mr);
    pilot->winnings = memread_udword(mr);
    pilot->total_value = memread_udword(mr);
    pilot->unk_f_a = memread_float(mr);
    pilot->unk_f_b = memread_float(mr);
    sd_mskip(mr, 8);
    palette_create(&pilot->palette);
    palette_mload_range(mr, &pilot->palette, 0, 48);
    pilot->unk_block_i = memread_uword(mr);

    pilot->photo_id = memread_uword(mr) & 0x3FF;
}

int sd_pilot_load(sd_reader *reader, sd_pilot *pilot) {
    if(reader == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    // Read block, XOR, Read to pilot, free memory
    memreader *mr = memreader_open_from_reader(reader, PILOT_BLOCK_LENGTH);
    memreader_xor(mr, PILOT_BLOCK_LENGTH & 0xFF);
    sd_pilot_load_from_mem(mr, pilot);
    memreader_close(mr);

    // Quote block
    for(int m = 0; m < 10; m++) {
        pilot->quotes[m] = sd_read_variable_str(reader);
    }
    return SD_SUCCESS;
}

void sd_pilot_save_player_to_mem(memwriter *w, const sd_pilot *pilot) {
    memwrite_buf(w, pilot->name, 18);
    memwrite_uword(w, pilot->wins);
    memwrite_uword(w, pilot->losses);
    memwrite_ubyte(w, pilot->rank);
    memwrite_ubyte(w, pilot->har_id);

    uint16_t stats_a = 0, stats_b = 0, stats_c = 0;
    uint8_t stats_d = 0;
    stats_a |= (pilot->arm_power & 0x1F) << 0;
    stats_a |= (pilot->leg_power & 0x1F) << 5;
    stats_a |= (pilot->arm_speed & 0x1F) << 10;
    stats_b |= (pilot->leg_speed & 0x1F) << 0;
    stats_b |= (pilot->armor & 0x1F) << 5;
    stats_b |= (pilot->stun_resistance & 0x1F) << 10;
    stats_c |= (pilot->agility & 0x7F) << 0;
    stats_c |= (pilot->power & 0x7F) << 7;
    stats_d |= (pilot->endurance & 0x7F) << 0;
    memwrite_uword(w, stats_a);
    memwrite_uword(w, stats_b);
    memwrite_uword(w, stats_c);
    memwrite_ubyte(w, stats_d);
    memwrite_fill(w, 0, 1);

    memwrite_uword(w, pilot->offense);
    memwrite_uword(w, pilot->defense);
    memwrite_dword(w, pilot->money);
    memwrite_ubyte(w, pilot->color_1);
    memwrite_ubyte(w, pilot->color_2);
    memwrite_ubyte(w, pilot->color_3);
}

void sd_pilot_save_to_mem(memwriter *w, const sd_pilot *pilot) {
    // Write the pilot block
    memwrite_udword(w, pilot->unknown_a);

    sd_pilot_save_player_to_mem(w, pilot);

    memwrite_buf(w, pilot->trn_name, 13);
    memwrite_buf(w, pilot->trn_desc, 31);
    memwrite_buf(w, pilot->trn_image, 13);

    memwrite_float(w, pilot->unk_f_c);
    memwrite_float(w, pilot->unk_f_d);
    memwrite_fill(w, 0, 40);
    memwrite_ubyte(w, pilot->pilot_id);
    memwrite_ubyte(w, pilot->unknown_k);
    memwrite_uword(w, pilot->force_arena);
    memwrite_ubyte(w, (pilot->difficulty & 0x3) << 3);
    memwrite_buf(w, pilot->unk_block_b, 2);
    memwrite_ubyte(w, pilot->movement);
    memwrite_buf(w, (char *)pilot->unk_block_c, 6);
    memwrite_buf(w, pilot->enhancements, 11);

    // Flags
    memwrite_ubyte(w, 0);
    uint8_t req_flags = 0;
    if(pilot->secret)
        req_flags |= 0x02;
    if(pilot->only_fight_once)
        req_flags |= 0x08;
    memwrite_ubyte(w, req_flags);
    memwrite_ubyte(w, 0);

    // Requirements (10)
    uint16_t reqs[5];
    memset((char *)reqs, 0, 10);
    reqs[0] |= (pilot->req_max_rank & 0xFF) << 8;
    reqs[0] |= (pilot->req_rank & 0xFF);
    reqs[1] |= (pilot->req_fighter & 0x1F);
    reqs[2] |= (pilot->req_difficulty & 0x0F) << 8;
    reqs[2] |= (pilot->req_enemy & 0xFF);
    reqs[3] |= (pilot->req_accuracy & 0x7F) << 7;
    reqs[3] |= (pilot->req_vitality & 0x7F);
    reqs[4] |= (pilot->req_destroy & 0x01) << 8;
    reqs[4] |= (pilot->req_scrap & 0x01) << 7;
    reqs[4] |= (pilot->req_avg_dmg & 0x7F);
    memwrite_buf(w, (char *)reqs, 10);

    // Attitude
    uint16_t att[3];
    memset((char *)att, 0, 6);
    att[0] |= (pilot->att_normal & 0x7F) << 4;
    att[1] |= (pilot->att_jump & 0x7F) << 7;
    att[1] |= (pilot->att_hyper & 0x7F);
    att[2] |= (pilot->att_sniper & 0x7F) << 7;
    att[2] |= (pilot->att_def & 0x7F);
    memwrite_buf(w, (char *)att, 6);

    memwrite_buf(w, (char *)pilot->unk_block_d, 6);

    memwrite_word(w, pilot->ap_throw);
    memwrite_word(w, pilot->ap_special);
    memwrite_word(w, pilot->ap_jump);
    memwrite_word(w, pilot->ap_high);
    memwrite_word(w, pilot->ap_low);
    memwrite_word(w, pilot->ap_middle);
    memwrite_word(w, pilot->pref_jump);
    memwrite_word(w, pilot->pref_fwd);
    memwrite_word(w, pilot->pref_back);

    memwrite_udword(w, pilot->unknown_e);
    memwrite_float(w, pilot->learning);
    memwrite_float(w, pilot->forget);
    memwrite_buf(w, pilot->unk_block_f, 14);
    memwrite_uword(w, pilot->enemies_inc_unranked);
    memwrite_uword(w, pilot->enemies_ex_unranked);
    memwrite_uword(w, pilot->unk_d_a);
    memwrite_udword(w, pilot->har_trades);
    memwrite_udword(w, pilot->winnings);
    memwrite_udword(w, pilot->total_value);
    memwrite_float(w, pilot->unk_f_a);
    memwrite_float(w, pilot->unk_f_b);
    memwrite_fill(w, 0, 8);
    palette_msave_range(w, &pilot->palette, 0, 48);
    memwrite_uword(w, pilot->unk_block_i);

    memwrite_uword(w, pilot->photo_id & 0x3FF);
}

int sd_pilot_save(sd_writer *fw, const sd_pilot *pilot) {
    if(fw == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    // Copy, XOR, save and close
    memwriter *w = memwriter_open();
    sd_pilot_save_to_mem(w, pilot);
    memwriter_xor(w, PILOT_BLOCK_LENGTH & 0xFF);
    memwriter_save(w, fw);
    memwriter_close(w);

    // Quote block
    for(int m = 0; m < 10; m++) {
        sd_write_variable_str(fw, pilot->quotes[m]);
    }
    return SD_SUCCESS;
}

void sd_pilot_set_player_color(sd_pilot *pilot, player_color index, uint8_t color) {
    switch(index) {
        case PRIMARY:
            pilot->color_3 = color;
            palette_set_player_color(&pilot->palette, 0, pilot->color_3, 0);
            break;
        case SECONDARY:
            pilot->color_2 = color;
            palette_set_player_color(&pilot->palette, 0, pilot->color_2, 1);
            break;
        case TERTIARY:
            pilot->color_1 = color;
            palette_set_player_color(&pilot->palette, 0, pilot->color_1, 2);
            break;
    }
}

void sd_pilot_exit_tournament(sd_pilot *pilot) {
    pilot->rank = 0;
    pilot->trn_name[0] = '\0';
    pilot->trn_desc[0] = '\0';
    pilot->trn_image[0] = '\0';
}
