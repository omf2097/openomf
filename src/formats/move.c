#include <stdlib.h>
#include <string.h>

#include "formats/animation.h"
#include "formats/error.h"
#include "formats/move.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"

int sd_move_create(sd_move *move) {
    if(move == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear everything
    memset(move, 0, sizeof(sd_move));
    return SD_SUCCESS;
}

int sd_move_copy(sd_move *dst, const sd_move *src) {
    int ret;
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear destination
    memset(dst, 0, sizeof(sd_move));

    // Copy animation
    if(src->animation != NULL) {
        dst->animation = omf_calloc(1, sizeof(sd_animation));
        if((ret = sd_animation_copy(dst->animation, src->animation)) != SD_SUCCESS) {
            return ret;
        }
    }

    // Copy move and footer strings
    strncpy(dst->move_string, src->move_string, sizeof(dst->move_string));
    strncpy(dst->footer_string, src->footer_string, sizeof(dst->footer_string));

    // Everything else
    dst->ai_opts = src->ai_opts;
    dst->pos_constraint = src->pos_constraint;
    dst->unknown_4 = src->unknown_4;
    dst->unknown_5 = src->unknown_5;
    dst->unknown_6 = src->unknown_6;
    dst->unknown_7 = src->unknown_7;
    dst->unknown_8 = src->unknown_8;
    dst->unknown_9 = src->unknown_9;
    dst->unknown_10 = src->unknown_10;
    dst->unknown_11 = src->unknown_11;
    dst->next_anim_id = src->next_anim_id;
    dst->category = src->category;
    dst->block_damage = src->block_damage;
    dst->block_stun = src->block_stun;
    dst->successor_id = src->successor_id;
    dst->damage_amount = src->damage_amount;
    dst->collision_opts = src->collision_opts;
    dst->extra_string_selector = src->extra_string_selector;
    dst->points = src->points;

    return SD_SUCCESS;
}

void sd_move_free(sd_move *move) {
    if(move == NULL)
        return;
    if(move->animation != NULL) {
        sd_animation_free(move->animation);
        omf_free(move->animation);
    }
}

int sd_move_load(sd_reader *r, sd_move *move) {
    int ret;
    uint16_t size;

    if(r == NULL || move == NULL) {
        return SD_INVALID_INPUT;
    }

    // Read animation
    move->animation = omf_calloc(1, sizeof(sd_animation));
    if((ret = sd_animation_create(move->animation)) != SD_SUCCESS) {
        return ret;
    }
    if((ret = sd_animation_load(r, move->animation)) != SD_SUCCESS) {
        return ret;
    }

    // Header
    move->ai_opts = sd_read_uword(r);
    move->pos_constraint = sd_read_uword(r);
    move->unknown_4 = sd_read_ubyte(r);
    move->unknown_5 = sd_read_ubyte(r);
    move->unknown_6 = sd_read_ubyte(r);
    move->unknown_7 = sd_read_ubyte(r);
    move->unknown_8 = sd_read_ubyte(r);
    move->unknown_9 = sd_read_ubyte(r);
    move->unknown_10 = sd_read_ubyte(r);
    move->unknown_11 = sd_read_ubyte(r);
    move->next_anim_id = sd_read_ubyte(r);
    move->category = sd_read_ubyte(r);
    move->block_damage = sd_read_ubyte(r);
    move->block_stun = sd_read_ubyte(r);
    move->successor_id = sd_read_ubyte(r);
    move->damage_amount = sd_read_ubyte(r);
    move->collision_opts = sd_read_ubyte(r);
    move->extra_string_selector = sd_read_ubyte(r);
    move->points = sd_read_ubyte(r);

    // move string
    sd_read_buf(r, move->move_string, 21);

    // Footer string
    size = sd_read_uword(r);
    if(size >= SD_MOVE_FOOTER_STRING_MAX) {
        log_debug("Move footer too big! Expected max %d bytes, got %hu bytes.", SD_MOVE_FOOTER_STRING_MAX, size);
        return SD_FILE_PARSE_ERROR;
    }
    if(size > 0) {
        sd_read_buf(r, move->footer_string, size);
        if(move->footer_string[size - 1] != 0) {
            return SD_FILE_PARSE_ERROR;
        }
    }

    // Return success if reader is still ok
    if(!sd_reader_ok(r)) {
        return SD_FILE_PARSE_ERROR;
    }
    return SD_SUCCESS;
}

int sd_move_save(sd_writer *w, const sd_move *move) {
    int ret;
    uint16_t size;

    if(w == NULL || move == NULL) {
        return SD_INVALID_INPUT;
    }

    // Save animation
    if((ret = sd_animation_save(w, move->animation)) != SD_SUCCESS) {
        return ret;
    }

    // Move header
    sd_write_uword(w, move->ai_opts);
    sd_write_uword(w, move->pos_constraint);
    sd_write_ubyte(w, move->unknown_4);
    sd_write_ubyte(w, move->unknown_5);
    sd_write_ubyte(w, move->unknown_6);
    sd_write_ubyte(w, move->unknown_7);
    sd_write_ubyte(w, move->unknown_8);
    sd_write_ubyte(w, move->unknown_9);
    sd_write_ubyte(w, move->unknown_10);
    sd_write_ubyte(w, move->unknown_11);
    sd_write_ubyte(w, move->next_anim_id);
    sd_write_ubyte(w, move->category);
    sd_write_ubyte(w, move->block_damage);
    sd_write_ubyte(w, move->block_stun);
    sd_write_ubyte(w, move->successor_id);
    sd_write_ubyte(w, move->damage_amount);
    sd_write_ubyte(w, move->collision_opts);
    sd_write_ubyte(w, move->extra_string_selector);
    sd_write_ubyte(w, move->points);

    // move string
    sd_write_buf(w, move->move_string, 21);

    // Save footer string
    size = strlen(move->footer_string);
    if(size > 0) {
        sd_write_uword(w, size + 1);
        sd_write_buf(w, move->footer_string, size + 1);
    } else {
        sd_write_uword(w, 0);
    }

    return SD_SUCCESS;
}

int sd_move_set_animation(sd_move *move, const sd_animation *animation) {
    int ret;
    if(move == NULL) {
        return SD_INVALID_INPUT;
    }
    if(move->animation != NULL) {
        sd_animation_free(move->animation);
        omf_free(move->animation);
    }
    if(animation == NULL) {
        return SD_SUCCESS;
    }
    move->animation = omf_calloc(1, sizeof(sd_animation));
    if((ret = sd_animation_copy(move->animation, animation)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_animation *sd_move_get_animation(const sd_move *move) {
    return move->animation;
}

int sd_move_set_footer_string(sd_move *move, const char *str) {
    if(strlen(str) >= SD_MOVE_FOOTER_STRING_MAX - 1) {
        return SD_INVALID_INPUT;
    }
    strncpy_or_truncate(move->footer_string, str, sizeof(move->footer_string));
    return SD_SUCCESS;
}

int sd_move_set_move_string(sd_move *move, const char *str) {
    if(strlen(str) >= SD_MOVE_STRING_MAX - 1) {
        return SD_INVALID_INPUT;
    }
    strncpy_or_truncate(move->move_string, str, sizeof(move->move_string));
    return SD_SUCCESS;
}
