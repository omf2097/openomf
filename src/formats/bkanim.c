#include <stdlib.h>
#include <string.h>

#include "formats/animation.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "utils/allocator.h"
#include "utils/c_string_util.h"
#include "utils/log.h"

int sd_bk_anim_create(sd_bk_anim *bka) {
    if(bka == NULL) {
        return SD_INVALID_INPUT;
    }
    // clear everything
    memset(bka, 0, sizeof(sd_bk_anim));
    return SD_SUCCESS;
}

int sd_bk_anim_copy(sd_bk_anim *dst, const sd_bk_anim *src) {
    int ret;
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear destination
    memset(dst, 0, sizeof(sd_bk_anim));

    // Basic stuff
    dst->null = src->null;
    dst->chain_hit = src->chain_hit;
    dst->chain_no_hit = src->chain_no_hit;
    dst->load_on_start = src->load_on_start;
    dst->probability = src->probability;
    dst->hazard_damage = src->hazard_damage;

    // Footer string
    strncpy(dst->footer_string, src->footer_string, sizeof(dst->footer_string));

    // Copy animation (if exists)
    if(src->animation != NULL) {
        dst->animation = omf_calloc(1, sizeof(sd_animation));
        if((ret = sd_animation_copy(dst->animation, src->animation)) != SD_SUCCESS) {
            return ret;
        }
    }

    return SD_SUCCESS;
}

void sd_bk_anim_free(sd_bk_anim *bka) {
    if(bka->animation != NULL) {
        sd_animation_free(bka->animation);
        omf_free(bka->animation);
    }
}

int sd_bk_anim_load(sd_reader *r, sd_bk_anim *bka) {
    int ret;
    uint16_t size;

    // BK Specific animation header
    bka->null = sd_read_ubyte(r);
    bka->chain_hit = sd_read_ubyte(r);
    bka->chain_no_hit = sd_read_ubyte(r);
    bka->load_on_start = sd_read_ubyte(r);
    bka->probability = sd_read_uword(r);
    bka->hazard_damage = sd_read_ubyte(r);

    // Footer string
    size = sd_read_uword(r);
    if(size >= SD_BK_FOOTER_STRING_MAX) {
        log_debug("BK specific animation footer too big! Expected max %d bytes, got %hu bytes.",
                  SD_BK_FOOTER_STRING_MAX, size);
        return SD_FILE_PARSE_ERROR;
    }
    if(size > 0) {
        sd_read_buf(r, bka->footer_string, size);
        if(bka->footer_string[size - 1] != 0) {
            return SD_FILE_PARSE_ERROR;
        }
    }

    // Initialize animation
    bka->animation = omf_calloc(1, sizeof(sd_animation));
    if((ret = sd_animation_create(bka->animation)) != SD_SUCCESS) {
        return ret;
    }
    if((ret = sd_animation_load(r, bka->animation)) != SD_SUCCESS) {
        return ret;
    }

    // Return success
    return SD_SUCCESS;
}

int sd_bk_anim_save(sd_writer *w, const sd_bk_anim *bka) {
    int ret;
    uint16_t size;

    if(w == NULL || bka == NULL) {
        return SD_INVALID_INPUT;
    }

    // Write BK specific header
    sd_write_ubyte(w, bka->null);
    sd_write_ubyte(w, bka->chain_hit);
    sd_write_ubyte(w, bka->chain_no_hit);
    sd_write_ubyte(w, bka->load_on_start);
    sd_write_uword(w, bka->probability);
    sd_write_ubyte(w, bka->hazard_damage);

    // Save footer string
    size = strlen(bka->footer_string);
    if(size > 0) {
        sd_write_uword(w, size + 1);
        sd_write_buf(w, bka->footer_string, size + 1);
    } else {
        sd_write_uword(w, 0);
    }

    // Write animation
    if((ret = sd_animation_save(w, bka->animation)) != SD_SUCCESS) {
        return ret;
    }

    return SD_SUCCESS;
}

int sd_bk_anim_set_animation(sd_bk_anim *bka, const sd_animation *animation) {
    int ret;
    if(bka == NULL) {
        return SD_INVALID_INPUT;
    }
    if(bka->animation != NULL) {
        sd_animation_free(bka->animation);
        omf_free(bka->animation);
    }
    if(animation == NULL) {
        return SD_SUCCESS;
    }
    bka->animation = omf_calloc(1, sizeof(sd_animation));
    if((ret = sd_animation_copy(bka->animation, animation)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

sd_animation *sd_bk_anim_get_animation(const sd_bk_anim *bka) {
    return bka->animation;
}

int sd_bk_set_anim_string(sd_bk_anim *bka, const char *data) {
    if(strlen(data) >= SD_BK_FOOTER_STRING_MAX - 1) {
        return SD_INVALID_INPUT;
    }
    strncpy_or_truncate(bka->footer_string, data, sizeof(bka->footer_string));
    return SD_SUCCESS;
}
