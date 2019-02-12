#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/error.h"
#include "shadowdive/rec.h"

int sd_rec_extra_len(int key) {
    switch(key) {
        case 2:
        case 3:
        case 5:
            return 1;
        case 6:
            return 60;
        case 10:
        case 18:
            return 8;
    }
    return 0;
}

int sd_rec_create(sd_rec_file *rec) {
    if(rec == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(rec, 0, sizeof(sd_rec_file));
    sd_pilot_create(&rec->pilots[0].info);
    sd_pilot_create(&rec->pilots[1].info);
    return SD_SUCCESS;
}

void sd_rec_free(sd_rec_file *rec) {
    if(rec == NULL) return;
    sd_pilot_free(&rec->pilots[0].info);
    sd_pilot_free(&rec->pilots[1].info);
    if(rec->moves) {
        for(int i = 0; i < rec->move_count; i++) {
            free(rec->moves[i].extra_data);
        }
        free(rec->moves);
    }
}

int sd_rec_load(sd_rec_file *rec, const char *file) {
    int ret = SD_FILE_PARSE_ERROR;
    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    sd_reader *r = sd_reader_open(file);
    if(!r) {
        return SD_FILE_OPEN_ERROR;
    }

    // Make sure we have at least this much data
    if(sd_reader_filesize(r) < 1224) {
        goto error_0;
    }

    // Read pilot data
    for(int i = 0; i < 2; i++) {
        // Read pilot data
        sd_pilot_create(&rec->pilots[i].info);
        if((ret = sd_pilot_load(r, &rec->pilots[i].info)) != SD_SUCCESS) { goto error_0; }
        rec->pilots[i].unknown_a = sd_read_ubyte(r);
        rec->pilots[i].unknown_b = sd_read_uword(r);
        sd_palette_create(&rec->pilots[i].pal);
        sd_palette_load_range(r, &rec->pilots[i].pal, 0, 48);
        rec->pilots[i].has_photo = sd_read_ubyte(r);
        sd_sprite_create(&rec->pilots[i].photo);
        if(rec->pilots[i].has_photo) {
            if((ret = sd_sprite_load(r, &rec->pilots[i].photo)) != SD_SUCCESS) {
                goto error_0;
            }
        }
    }

    // Scores
    for(int i = 0; i < 2; i++)
        rec->scores[i] = sd_read_udword(r);

    // Other flags
    rec->unknown_a = sd_read_byte(r);
    rec->unknown_b = sd_read_byte(r);
    rec->unknown_c = sd_read_byte(r);
    rec->throw_range = sd_read_word(r);
    rec->hit_pause = sd_read_word(r);
    rec->block_damage = sd_read_word(r);
    rec->vitality = sd_read_word(r);
    rec->jump_height = sd_read_word(r);
    rec->unknown_i = sd_read_word(r);
    rec->unknown_j = sd_read_word(r);
    rec->unknown_k = sd_read_word(r);
    uint32_t in = sd_read_udword(r);
    rec->knock_down = (in >> 0 ) & 0x03; // 00000000 00000000 00000000 00000011 (2)
    rec->rehit_mode = (in >> 2 ) & 0x01; // 00000000 00000000 00000000 00000100 (1)
    rec->def_throws = (in >> 3 ) & 0x01; // 00000000 00000000 00000000 00001000 (1)
    rec->arena_id =   (in >> 4 ) & 0x1F; // 00000000 00000000 00000001 11110000 (5)
    rec->power[0] =   (in >> 9 ) & 0x1F; // 00000000 00000000 00111110 00000000 (5)
    rec->power[1] =   (in >> 14) & 0x1F; // 00000000 00000111 11000000 00000000 (5)
    rec->hazards =    (in >> 19) & 0x01; // 00000000 00001000 00000000 00000000 (1)
    rec->round_type = (in >> 20) & 0x03; // 00000000 00110000 00000000 00000000 (2)
    rec->unknown_l =  (in >> 22) & 0x03; // 00000000 11000000 00000000 00000000 (2)
    rec->hyper_mode = (in >> 24) & 0x01; // 00000001 00000000 00000000 00000000 (1)
    rec->unknown_m = sd_read_byte(r);

    // Allocate enough space for the record blocks
    // This will be reduced later when we know the ACTUAL count
    size_t rsize = sd_reader_filesize(r) - sd_reader_pos(r);
    rec->move_count = rsize / 7;
    rec->moves = calloc(rec->move_count, sizeof(sd_rec_move));

    // Read blocks
    for(int i = 0; i < rec->move_count; i++) {
        rec->moves[i].tick = sd_read_udword(r);
        rec->moves[i].lookup_id = sd_read_ubyte(r);
        rec->moves[i].player_id = sd_read_ubyte(r);
        int extra_length = sd_rec_extra_len(rec->moves[i].lookup_id);
        if(extra_length > 0) {
            uint8_t action = sd_read_ubyte(r);
            rec->moves[i].raw_action = action;

            // Parse real action key
            rec->moves[i].action = SD_ACT_NONE;
            if(action & 1) {
                rec->moves[i].action |= SD_ACT_PUNCH;
            }
            if(action & 2) {
                rec->moves[i].action |= SD_ACT_KICK;
            }
            switch(action & 0xF0) {
                case 16: rec->moves[i].action |= SD_ACT_UP; break;
                case 32: rec->moves[i].action |= (SD_ACT_UP|SD_ACT_RIGHT); break;
                case 48: rec->moves[i].action |= SD_ACT_RIGHT; break;
                case 64: rec->moves[i].action |= (SD_ACT_DOWN|SD_ACT_RIGHT); break;
                case 80: rec->moves[i].action |= SD_ACT_DOWN; break;
                case 96: rec->moves[i].action |= (SD_ACT_DOWN|SD_ACT_LEFT); break;
                case 112: rec->moves[i].action |= SD_ACT_LEFT; break;
                case 128: rec->moves[i].action |= (SD_ACT_UP|SD_ACT_LEFT); break;
            }

            // We already read the action key, so minus one.
            int unknown_len = extra_length - 1;
            if(unknown_len > 0) {
                rec->moves[i].extra_data = malloc(unknown_len);
                sd_read_buf(r, rec->moves[i].extra_data, unknown_len);
                rec->move_count--;
            }
        }
    }

    // Okay, not reduce the allocated memory to match what we actually need
    // Realloc should keep our old data intact
    rec->moves = realloc(rec->moves, rec->move_count * sizeof(sd_rec_move));

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_0:
    sd_reader_close(r);
    return ret;
}

int sd_rec_save(sd_rec_file *rec, const char *file) {
    sd_writer *w;

    if(rec == NULL || file == NULL) {
        return SD_INVALID_INPUT;
    }

    if(!(w = sd_writer_open(file))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write pilots, palettes, etc.
    for(int i = 0; i < 2; i++) {
        sd_pilot_save(w, &rec->pilots[i].info);
        sd_write_ubyte(w, rec->pilots[i].unknown_a);
        sd_write_uword(w, rec->pilots[i].unknown_b);
        sd_palette_save_range(w, &rec->pilots[i].pal, 0, 48);
        sd_write_ubyte(w, rec->pilots[i].has_photo);
        if(rec->pilots[i].has_photo) {
            sd_sprite_save(w, &rec->pilots[i].photo);
        }
    }

    // Scores
    for(int i = 0; i < 2; i++)
        sd_write_udword(w, rec->scores[i]);

    // Other header data
    sd_write_byte(w, rec->unknown_a);
    sd_write_byte(w, rec->unknown_b);
    sd_write_byte(w, rec->unknown_c);
    sd_write_word(w, rec->throw_range);
    sd_write_word(w, rec->hit_pause);
    sd_write_word(w, rec->block_damage);
    sd_write_word(w, rec->vitality);
    sd_write_word(w, rec->jump_height);
    sd_write_word(w, rec->unknown_i);
    sd_write_word(w, rec->unknown_j);
    sd_write_word(w, rec->unknown_k);
    uint32_t out = 0;
    out |= (rec->knock_down & 0x3) << 0;
    out |= (rec->rehit_mode & 0x1) << 2;
    out |= (rec->def_throws & 0x1) << 3;
    out |= (rec->arena_id & 0x1F) << 4;
    out |= (rec->power[0] & 0x1F) << 9;
    out |= (rec->power[1] & 0x1F) << 14;
    out |= (rec->hazards & 0x1) << 19;
    out |= (rec->round_type & 0x3) << 20;
    out |= (rec->unknown_l & 0x3) << 22;
    out |= (rec->hyper_mode & 0x1) << 24;
    sd_write_udword(w, out);
    sd_write_byte(w, rec->unknown_m);

    // Move records
    for(int i = 0; i < rec->move_count; i++) {
        sd_write_udword(w, rec->moves[i].tick);
        sd_write_ubyte(w, rec->moves[i].lookup_id);
        sd_write_ubyte(w, rec->moves[i].player_id);

        int extra_length = sd_rec_extra_len(rec->moves[i].lookup_id);
        if(extra_length > 0) {
            // Write action information
            uint8_t raw_action = 0;
            switch(rec->moves[i].action & SD_MOVE_MASK) {
                case (SD_ACT_UP): raw_action = 16; break;
                case (SD_ACT_UP|SD_ACT_RIGHT): raw_action = 32; break;
                case (SD_ACT_RIGHT): raw_action = 48; break;
                case (SD_ACT_DOWN|SD_ACT_RIGHT): raw_action = 64; break;
                case (SD_ACT_DOWN): raw_action = 80; break;
                case (SD_ACT_DOWN|SD_ACT_LEFT): raw_action = 96; break;
                case (SD_ACT_LEFT): raw_action = 112; break;
                case (SD_ACT_UP|SD_ACT_LEFT): raw_action = 128; break;
            }
            if(rec->moves[i].action & SD_ACT_PUNCH)
                raw_action |= 1;
            if(rec->moves[i].action & SD_ACT_KICK)
                raw_action |= 2;
            sd_write_ubyte(w, raw_action);

            // If there is more extra data, write it
            int unknown_len = extra_length - 1;
            if(unknown_len > 0) {
                sd_write_buf(w, rec->moves[i].extra_data, unknown_len);
            }
        }
   }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_rec_delete_action(sd_rec_file *rec, unsigned int number) {
    if(rec == NULL || number >= rec->move_count) {
        return SD_INVALID_INPUT;
    }

    // Only move if we are not deleting the last entry
    if(number < (rec->move_count - 1)) {
        memmove(
            rec->moves + number,
            rec->moves + number + 1,
            (rec->move_count - number - 1) * sizeof(sd_rec_move));
    }

    // Resize to save memory
    rec->move_count--;
    rec->moves = realloc(rec->moves, rec->move_count * sizeof(sd_rec_move));

    if(rec->moves == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    return SD_SUCCESS;
}

int sd_rec_insert_action(sd_rec_file *rec, unsigned int number, const sd_rec_move *move) {
    if(rec == NULL) {
        return SD_INVALID_INPUT;
    }
    if(number > rec->move_count) {
        return SD_INVALID_INPUT;
    }

    // Resize
    rec->moves = realloc(rec->moves, (rec->move_count+1) * sizeof(sd_rec_move));
    if(rec->moves == NULL) {
        return SD_OUT_OF_MEMORY;
    }

    // Only move if we are inserting, not appending
    // when number == move_count-1, we are pushing the last entry forwards by one
    // when number == move_count, we are pushing to the end.
    if(number < rec->move_count) {
        memmove(
            rec->moves + number + 1,
            rec->moves + number, 
            (rec->move_count - number) * sizeof(sd_rec_move));
    }
    memcpy(
        rec->moves + number,
        move,
        sizeof(sd_rec_move));

    rec->move_count++;
    return SD_SUCCESS;
}
