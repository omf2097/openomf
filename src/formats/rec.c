#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/internal/reader.h"
#include "formats/internal/writer.h"
#include "formats/rec.h"
#include "utils/allocator.h"

int sd_rec_extra_len(int key) {
    assert(0 <= key && key < 192);
    // Do not edit these values, these must match the original
    // game's lookup table.
    switch(key) {
        case 0:
        case 11:
        case 12:
        case 16:
        case 17:
        case 19:
            return 0;
        case 14:
            return 2;
        case 21:
            return 3;
        case 7:
            return 4;
        case 4:
        case 10:
        case 18:
            return 8;
        case 15:
            return 20;
        case 6:
            return 60;
        case 20:
            return 144;
    }
    return key < 0x60 ? 1 : 10;
}

char *sd_rec_get_extra_data(sd_rec_move *move) {
    return smallbuffer_data(&move->extra_data);
}

char *sd_rec_set_lookup_id(sd_rec_move *move, int key) {
    if(key == move->lookup_id) {
        return smallbuffer_data(&move->extra_data);
    }
    move->lookup_id = key;
    size_t len = sd_rec_extra_len(key);
    smallbuffer_realloc_with_custom_selfsize(&move->extra_data, len,
                                             sizeof(move->extra_data) + sizeof(move->extra_data_padding));
    if(len != 0) {
        memset(smallbuffer_data(&move->extra_data), 0, len);
    }
    return smallbuffer_data(&move->extra_data);
}

void sd_rec_move_free(sd_rec_move *move) {
    smallbuffer_free(&move->extra_data);
    move->lookup_id = 0;
}

static void sd_rec_move_free_cb(void *ptr) {
    sd_rec_move_free(ptr);
}

void sd_rec_create(sd_rec_file *rec) {
    assert(rec != NULL);
    memset(rec, 0, sizeof(sd_rec_file));
    vector_create_cb(&rec->moves, sizeof(sd_rec_move), sd_rec_move_free_cb);
    sd_pilot_create(&rec->pilots[0].info);
    sd_pilot_create(&rec->pilots[1].info);
}

void sd_rec_free(sd_rec_file *rec) {
    if(rec == NULL) {
        return;
    }
    // The vector's free callback frees each move's extra data.
    vector_free(&rec->moves);

    for(int i = 0; i < 2; i++) {
        sd_pilot_free(&rec->pilots[i].info);
    }
    // zero it out so we can pass this so sd_rec_create can be used again
    memset(rec, 0, sizeof(sd_rec_file));
}

int sd_rec_load(sd_rec_file *rec, const path *file) {
    int ret = SD_FILE_PARSE_ERROR;
    assert(rec != NULL);
    assert(file != NULL);

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
        if((ret = sd_pilot_load(r, &rec->pilots[i].info)) != SD_SUCCESS) {
            goto error_0;
        }
        rec->pilots[i].unknown_a = sd_read_ubyte(r);
        rec->pilots[i].unknown_b = sd_read_uword(r);
        vga_palette_init(&rec->pilots[i].info.palette);
        palette_load_range(r, &rec->pilots[i].info.palette, 0, 48);
        if(sd_read_ubyte(r)) {
            rec->pilots[i].info.photo = omf_calloc(1, sizeof(sd_sprite));
            sd_sprite_create(rec->pilots[i].info.photo);
            ret = sd_sprite_load(r, rec->pilots[i].info.photo);
            if(ret != SD_SUCCESS) {
                goto error_0;
            }
        }
    }

    // Scores
    for(int i = 0; i < 2; i++) {
        rec->scores[i] = sd_read_udword(r);
    }

    // Other flags
    rec->unknown_a = sd_read_byte(r);
    rec->arena_palette = sd_read_byte(r);
    rec->game_mode = sd_read_byte(r);
    rec->throw_range = sd_read_word(r);
    rec->hit_pause = sd_read_word(r);
    rec->block_damage = sd_read_word(r);
    rec->vitality = sd_read_word(r);
    rec->jump_height = sd_read_word(r);
    rec->p1_controller = sd_read_word(r);
    rec->p2_controller = sd_read_word(r);
    rec->p2_controller_ = sd_read_word(r);
    uint32_t in = sd_read_udword(r);
    rec->knock_down = (in >> 0) & 0x03;  // 00000000 00000000 00000000 00000011 (2)
    rec->rehit_mode = (in >> 2) & 0x01;  // 00000000 00000000 00000000 00000100 (1)
    rec->def_throws = (in >> 3) & 0x01;  // 00000000 00000000 00000000 00001000 (1)
    rec->arena_id = (in >> 4) & 0x1F;    // 00000000 00000000 00000001 11110000 (5)
    rec->power[0] = (in >> 9) & 0x1F;    // 00000000 00000000 00111110 00000000 (5)
    rec->power[1] = (in >> 14) & 0x1F;   // 00000000 00000111 11000000 00000000 (5)
    rec->hazards = (in >> 19) & 0x01;    // 00000000 00001000 00000000 00000000 (1)
    rec->round_type = (in >> 20) & 0x03; // 00000000 00110000 00000000 00000000 (2)
    rec->unknown_l = (in >> 22) & 0x03;  // 00000000 11000000 00000000 00000000 (2)
    rec->hyper_mode = (in >> 24) & 0x01; // 00000001 00000000 00000000 00000000 (1)
    rec->unknown_m = sd_read_byte(r);

    // Reserve enough space for the record blocks up front
    const size_t rsize = sd_reader_filesize(r) - sd_reader_pos(r);
    const unsigned max_movecount = rsize / 6;
    vector_reserve(&rec->moves, max_movecount);

    // Read blocks
    while(vector_size(&rec->moves) < max_movecount && sd_reader_pos(r) < sd_reader_filesize(r)) {
        sd_rec_move *move = vector_append_ptr(&rec->moves);
        memset(move, 0, sizeof(sd_rec_move));
        move->tick = sd_read_udword(r);
        char *extra_data = sd_rec_set_lookup_id(move, sd_read_ubyte(r));
        move->player_id = sd_read_ubyte(r);
        int extra_length = sd_rec_extra_len(move->lookup_id);
        if(extra_length > 0) {
            sd_read_buf(r, extra_data, extra_length);
        }
    }

    // Close & return
    sd_reader_close(r);
    return SD_SUCCESS;

error_0:
    sd_reader_close(r);
    return ret;
}

int sd_rec_save(const sd_rec_file *rec, const path *file) {
    sd_writer *w;

    assert(rec != NULL);
    assert(file != NULL);

    if(!(w = sd_writer_open(file))) {
        return SD_FILE_OPEN_ERROR;
    }

    // Write pilots, palettes, etc.
    for(int i = 0; i < 2; i++) {
        sd_pilot_save(w, &rec->pilots[i].info);
        sd_write_ubyte(w, rec->pilots[i].unknown_a);
        sd_write_uword(w, rec->pilots[i].unknown_b);
        palette_save_range(w, &rec->pilots[i].info.palette, 0, 48);
        sd_write_ubyte(w, rec->pilots[i].info.photo ? 1 : 0);
        if(rec->pilots[i].info.photo) {
            sd_sprite_save(w, rec->pilots[i].info.photo);
        }
    }

    // Scores
    for(int i = 0; i < 2; i++) {
        sd_write_udword(w, rec->scores[i]);
    }

    // Other header data
    sd_write_byte(w, rec->unknown_a);
    sd_write_byte(w, rec->arena_palette);
    sd_write_byte(w, rec->game_mode);
    sd_write_word(w, rec->throw_range);
    sd_write_word(w, rec->hit_pause);
    sd_write_word(w, rec->block_damage);
    sd_write_word(w, rec->vitality);
    sd_write_word(w, rec->jump_height);
    sd_write_word(w, rec->p1_controller);
    sd_write_word(w, rec->p2_controller);
    sd_write_word(w, rec->p2_controller_);
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
    iterator it;
    sd_rec_move *move;
    vector_iter_begin(&rec->moves, &it);
    foreach(it, move) {
        sd_write_udword(w, move->tick);
        sd_write_ubyte(w, move->lookup_id);
        sd_write_ubyte(w, move->player_id);
        sd_write_buf(w, sd_rec_get_extra_data(move), sd_rec_extra_len(move->lookup_id));
    }

    sd_writer_close(w);
    return SD_SUCCESS;
}

int sd_rec_delete_action(sd_rec_file *rec, unsigned int number) {
    assert(rec != NULL);
    if(vector_delete_at(&rec->moves, number) != 0) {
        return SD_INVALID_INPUT;
    }
    return SD_SUCCESS;
}

int sd_rec_insert_action_at_tick(sd_rec_file *rec, sd_rec_move *move) {
    unsigned int i;
    for(i = 0; i < vector_size(&rec->moves); i++) {
        const sd_rec_move *m = vector_get(&rec->moves, i);
        if(move->tick < m->tick) {
            break;
        }
    }
    return sd_rec_insert_action(rec, i, move);
}

int sd_rec_insert_action(sd_rec_file *rec, unsigned int number, sd_rec_move *move) {
    assert(rec != NULL);
    if(vector_insert_at(&rec->moves, number, move) != 0) {
        return SD_INVALID_INPUT;
    }

    move->lookup_id = 0;
    memset(&move->extra_data, 0, sizeof(smallbuffer));
    return SD_SUCCESS;
}

void sd_rec_finish(sd_rec_file *rec, unsigned int ticks) {
    sd_rec_move move = {0};
    move.tick = ticks;
    move.player_id = 0;
    char *extra_data = sd_rec_set_lookup_id(&move, 2);
    extra_data[0] = SD_ACT_NONE;
    vector_append(&rec->moves, &move);
}
