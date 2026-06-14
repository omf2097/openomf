#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "formats/animation.h"
#include "formats/bkanim.h"
#include "formats/error.h"
#include "utils/allocator.h"
#include "utils/log.h"

int sd_bk_anim_create(sd_bk_anim *bka) {
    assert(bka != NULL);
    // clear everything
    memset(bka, 0, sizeof(sd_bk_anim));
    str_create(&bka->footer_string);
    return SD_SUCCESS;
}

int sd_bk_anim_copy(sd_bk_anim *dst, const sd_bk_anim *src) {
    int ret;
    assert(dst != NULL);
    assert(src != NULL);

    // Clear destination
    memset(dst, 0, sizeof(sd_bk_anim));

    // Basic stuff
    dst->null = src->null;
    dst->chain_hit = src->chain_hit;
    dst->chain_no_hit = src->chain_no_hit;
    dst->repeat = src->repeat;
    dst->probability = src->probability;
    dst->hazard_damage = src->hazard_damage;

    // Footer string
    str_from(&dst->footer_string, &src->footer_string);

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
    str_free(&bka->footer_string);
}

int sd_bk_anim_load(sd_reader *r, sd_bk_anim *bka) {
    int ret;

    // BK Specific animation header
    bka->null = sd_read_ubyte(r);
    bka->chain_hit = sd_read_ubyte(r);
    bka->chain_no_hit = sd_read_ubyte(r);
    bka->repeat = sd_read_ubyte(r);
    bka->probability = sd_read_uword(r);
    bka->hazard_damage = sd_read_ubyte(r);

    // Footer string
    if(!sd_read_padded_str(r, &bka->footer_string, SD_BK_FOOTER_STRING_MAX)) {
        log_debug("BK specific animation footer too big! Expected max %d bytes.", SD_BK_FOOTER_STRING_MAX);
        return SD_FILE_PARSE_ERROR;
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

    assert(w != NULL);
    assert(bka != NULL);

    // Write BK specific header
    sd_write_ubyte(w, bka->null);
    sd_write_ubyte(w, bka->chain_hit);
    sd_write_ubyte(w, bka->chain_no_hit);
    sd_write_ubyte(w, bka->repeat);
    sd_write_uword(w, bka->probability);
    sd_write_ubyte(w, bka->hazard_damage);

    // Save footer string
    sd_write_padded_str(w, &bka->footer_string);

    // Write animation
    if((ret = sd_animation_save(w, bka->animation)) != SD_SUCCESS) {
        return ret;
    }

    return SD_SUCCESS;
}

int sd_bk_anim_set_animation(sd_bk_anim *bka, const sd_animation *animation) {
    int ret;
    assert(bka != NULL);
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
