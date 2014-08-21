#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/memreader.h"
#include "shadowdive/internal/memwriter.h"
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

int sd_pilot_load(sd_reader *reader, sd_pilot *pilot) {
    if(reader == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    // Open memory reader and XOR 
    sd_mreader *mr = sd_mreader_open_from_reader(reader, PILOT_BLOCK_LENGTH);
    sd_mreader_xor(mr, PILOT_BLOCK_LENGTH & 0xFF);

    // Set vars 
    pilot->unknown_a =   sd_mread_udword(mr);
    sd_mread_buf(mr, pilot->name, 18);
    pilot->wins =        sd_mread_uword(mr);
    pilot->losses =      sd_mread_uword(mr);
    pilot->robot_id =    sd_mread_uword(mr);
    sd_mread_buf(mr, pilot->stats, 8);

    pilot->offense =     sd_mread_uword(mr);
    pilot->defense =     sd_mread_uword(mr);
    pilot->money =       sd_mread_udword(mr);
    pilot->color_1 =     sd_mread_ubyte(mr);
    pilot->color_2 =     sd_mread_ubyte(mr);
    pilot->color_3 =     sd_mread_ubyte(mr);

    sd_mread_buf(mr, pilot->unk_block_a, 107);
    pilot->force_arena = sd_mread_uword(mr);
    sd_mread_buf(mr, pilot->unk_block_b, 3);
    pilot->movement =    sd_mread_ubyte(mr);
    sd_mread_buf(mr, pilot->unk_block_c, 6);
    sd_mread_buf(mr, pilot->enhancements, 11);

    pilot->unk_flag_a =  sd_mread_ubyte(mr);
    pilot->flags =       sd_mread_ubyte(mr);
    pilot->unk_flag_b =  sd_mread_ubyte(mr);
    sd_mread_buf(mr, (char*)pilot->reqs, 10);
    sd_mread_buf(mr, (char*)pilot->attitude, 6);
    sd_mread_buf(mr, pilot->unk_block_d, 6);

    pilot->ap_throw =    sd_mread_uword(mr);
    pilot->ap_special =  sd_mread_uword(mr);
    pilot->ap_jump =     sd_mread_uword(mr);
    pilot->ap_high =     sd_mread_uword(mr);
    pilot->ap_low =      sd_mread_uword(mr);
    pilot->ap_middle =   sd_mread_uword(mr);
    pilot->pref_jump =   sd_mread_uword(mr);
    pilot->pref_fwd =    sd_mread_uword(mr);
    pilot->pref_back =   sd_mread_uword(mr);

    sd_mread_buf(mr, pilot->unk_block_e, 4);
    pilot->learning =    sd_mread_float(mr);
    pilot->forget =      sd_mread_float(mr);
    sd_mread_buf(mr, pilot->unk_block_f, 24);
    pilot->winnings =    sd_mread_udword(mr);
    sd_mread_buf(mr, pilot->unk_block_g, 166);
    pilot->photo_id =    sd_mread_uword(mr);

    // Close memory buffer reader
    sd_mreader_close(mr);
    return SD_SUCCESS;
}

int sd_pilot_save(sd_writer *fw, const sd_pilot *pilot) {
    if(fw == NULL || pilot == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_mwriter *w = sd_mwriter_open();

    // Write the pilot block
    sd_mwrite_udword(w, pilot->unknown_a);
    sd_mwrite_buf(w, pilot->name, 18);
    sd_mwrite_uword(w, pilot->wins);
    sd_mwrite_uword(w, pilot->losses);
    sd_mwrite_uword(w, pilot->robot_id);
    sd_mwrite_buf(w, pilot->stats, 8);

    sd_mwrite_uword(w, pilot->offense);
    sd_mwrite_uword(w, pilot->defense);
    sd_mwrite_udword(w, pilot->money);
    sd_mwrite_ubyte(w, pilot->color_1);
    sd_mwrite_ubyte(w, pilot->color_2);
    sd_mwrite_ubyte(w, pilot->color_3);

    sd_mwrite_buf(w, pilot->unk_block_a, 107);
    sd_mwrite_uword(w, pilot->force_arena);
    sd_mwrite_buf(w, pilot->unk_block_b, 3);
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

    sd_mwrite_buf(w, pilot->unk_block_e, 4);
    sd_mwrite_float(w, pilot->learning);
    sd_mwrite_float(w, pilot->forget);
    sd_mwrite_buf(w, pilot->unk_block_f, 24);
    sd_mwrite_udword(w, pilot->winnings);
    sd_mwrite_buf(w, pilot->unk_block_g, 166);
    sd_mwrite_uword(w, pilot->photo_id);

    // XOR block, save and close
    sd_mwriter_xor(w, PILOT_BLOCK_LENGTH & 0xFF);
    sd_mwriter_save(w, fw);
    sd_mwriter_close(w);
    return SD_SUCCESS;
}