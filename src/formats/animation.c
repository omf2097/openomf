#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "formats/animation.h"
#include "formats/colcoord.h"
#include "formats/error.h"
#include "formats/sprite.h"
#include "utils/allocator.h"
#include "utils/log.h"

int sd_animation_create(sd_animation *ani) {
    assert(ani != NULL);
    memset(ani, 0, sizeof(sd_animation));
    vector_create(&ani->coord_table, sizeof(sd_coord));
    vector_create_cb(&ani->extra_strings, sizeof(str), str_free_cb);
    str_create(&ani->anim_string);
    return SD_SUCCESS;
}

int sd_animation_copy(sd_animation *dst, const sd_animation *src) {
    int ret;
    assert(dst != NULL);
    assert(src != NULL);

    // Clear destination
    memset(dst, 0, sizeof(sd_animation));

    // Copy source
    dst->start_x = src->start_x;
    dst->start_y = src->start_y;
    dst->null = src->null;
    dst->sprite_count = src->sprite_count;

    // Copy animation string
    str_from(&dst->anim_string, &src->anim_string);

    // Copy col coordinates
    vector_clone(&dst->coord_table, &src->coord_table);

    // Deep copy extra strings
    vector_create_cb(&dst->extra_strings, sizeof(str), str_free_cb);
    iterator it;
    const str *src_str;
    vector_iter_begin(&src->extra_strings, &it);
    foreach(it, src_str) {
        str_from(vector_append_ptr(&dst->extra_strings), src_str);
    }

    // Copy sprites
    for(int i = 0; i < SD_SPRITE_COUNT_MAX; i++) {
        if(src->sprites[i] != NULL) {
            dst->sprites[i] = omf_calloc(1, sizeof(sd_sprite));
            if((ret = sd_sprite_copy(dst->sprites[i], src->sprites[i])) != SD_SUCCESS) {
                return ret;
            }
        }
    }
    return SD_SUCCESS;
}

void sd_animation_free(sd_animation *anim) {
    for(int i = 0; i < SD_SPRITE_COUNT_MAX; i++) {
        if(anim->sprites[i] != NULL) {
            sd_sprite_free(anim->sprites[i]);
            omf_free(anim->sprites[i]);
        }
    }
    vector_free(&anim->coord_table);

    // The vector's free callback frees each str element.
    vector_free(&anim->extra_strings);

    str_free(&anim->anim_string);
}

int sd_animation_get_sprite_count(const sd_animation *anim) {
    return anim->sprite_count;
}

int sd_animation_set_sprite(sd_animation *anim, int num, const sd_sprite *sprite) {
    int ret;
    assert(sprite != NULL);
    if(num < 0 || num >= anim->sprite_count) {
        return SD_INVALID_INPUT;
    }

    if(anim->sprites[num] != NULL) {
        sd_sprite_free(anim->sprites[num]);
        omf_free(anim->sprites[num]);
    }
    anim->sprites[num] = omf_calloc(1, sizeof(sd_sprite));
    if((ret = sd_sprite_copy(anim->sprites[num], sprite)) != SD_SUCCESS) {
        return ret;
    }
    return SD_SUCCESS;
}

int sd_animation_push_sprite(sd_animation *anim, const sd_sprite *sprite) {
    int ret;
    if(anim->sprite_count >= SD_SPRITE_COUNT_MAX) {
        return SD_INVALID_INPUT;
    }
    anim->sprites[anim->sprite_count] = omf_calloc(1, sizeof(sd_sprite));
    if((ret = sd_sprite_copy(anim->sprites[anim->sprite_count], sprite)) != SD_SUCCESS) {
        return ret;
    }
    anim->sprite_count++;
    return SD_SUCCESS;
}

int sd_animation_pop_sprite(sd_animation *anim) {
    if(anim->sprite_count <= 0) {
        return SD_INVALID_INPUT;
    }

    anim->sprite_count--;
    sd_sprite_free(anim->sprites[anim->sprite_count]);
    omf_free(anim->sprites[anim->sprite_count]);

    return SD_SUCCESS;
}

sd_sprite *sd_animation_get_sprite(sd_animation *anim, int num) {
    if(num < 0 || num >= SD_SPRITE_COUNT_MAX) {
        return NULL;
    }
    return anim->sprites[num];
}

int sd_animation_load(sd_reader *r, sd_animation *ani) {
    int ret;

    // Animation header
    ani->start_x = sd_read_word(r);
    ani->start_y = sd_read_word(r);
    ani->null = sd_read_udword(r);
    const uint16_t coord_count = sd_read_uword(r);
    ani->sprite_count = sd_read_ubyte(r);

    // Enforce limits
    if(coord_count > SD_COLCOORD_COUNT_MAX) {
        log_debug("Animation contains too many coordinates! Expected max %d coords, got %hu coords.",
                  SD_COLCOORD_COUNT_MAX, coord_count);
        return SD_FILE_PARSE_ERROR;
    }

    // Read collision point data
    for(int i = 0; i < coord_count; i++) {
        const uint32_t tmp = sd_read_udword(r);
        const uint16_t a = tmp & 0xffff;
        const uint16_t b = (tmp & 0xffff0000) >> 16;
        // Extract 10 bit signed integers to x and y
        sd_coord *coord = vector_append_ptr(&ani->coord_table);
        coord->x = ((a & 0x3ff) ^ 0x200) - 0x200;
        coord->null = (a >> 10);
        coord->y = ((b & 0x3ff) ^ 0x200) - 0x200;
        coord->frame_id = (b >> 10);
    }

    // Animation string header
    if(!sd_read_terminated_str(r, &ani->anim_string, SD_ANIMATION_STRING_MAX)) {
        log_debug("Animation string header too long or not null terminated!");
        return SD_FILE_PARSE_ERROR;
    }

    // Extra animation strings
    const uint8_t extra_string_count = sd_read_ubyte(r);
    if(extra_string_count > SD_EXTRASTR_COUNT_MAX) {
        log_debug("Animation has too many extra strings! Expected max %d strings, got %hhu strings.",
                  SD_EXTRASTR_COUNT_MAX, extra_string_count);
        return SD_FILE_PARSE_ERROR;
    }
    for(int i = 0; i < extra_string_count; i++) {
        str extra_string;
        if(!sd_read_terminated_str(r, &extra_string, SD_EXTRA_STRING_MAX)) {
            log_debug("Animation extra string %d too long or not null terminated!", i);
            return SD_FILE_PARSE_ERROR;
        }
        vector_append(&ani->extra_strings, &extra_string);
    }

    // Sprites
    for(int i = 0; i < ani->sprite_count; i++) {
        ani->sprites[i] = omf_calloc(1, sizeof(sd_sprite));
        if((ret = sd_sprite_create(ani->sprites[i])) != SD_SUCCESS) {
            return ret;
        }
        if((ret = sd_sprite_load(r, ani->sprites[i])) != SD_SUCCESS) {
            return ret;
        }
    }

    // Return success
    return SD_SUCCESS;
}

int sd_animation_save(sd_writer *w, const sd_animation *ani) {
    int ret;

    assert(ani != NULL);
    assert(w != NULL);

    // Animation header
    sd_write_word(w, ani->start_x);
    sd_write_word(w, ani->start_y);
    sd_write_udword(w, ani->null);
    sd_write_uword(w, vector_size(&ani->coord_table));
    sd_write_ubyte(w, ani->sprite_count);

    // collision table
    iterator it;
    const sd_coord *coord;
    vector_iter_begin(&ani->coord_table, &it);
    foreach(it, coord) {
        uint32_t tmp = (coord->frame_id & 0x3f);
        tmp = tmp << 10;
        tmp = (tmp | (coord->y & 0x3ff));
        tmp = tmp << 6;
        tmp = (tmp | (coord->null & 0x3f));
        tmp = tmp << 10;
        tmp = (tmp | (coord->x & 0x3ff));
        sd_write_udword(w, tmp);
    }

    // Animation string header
    sd_write_terminated_str(w, &ani->anim_string);

    // Extra animation strings
    sd_write_ubyte(w, vector_size(&ani->extra_strings));
    const str *extra_string;
    vector_iter_begin(&ani->extra_strings, &it);
    foreach(it, extra_string) {
        sd_write_terminated_str(w, extra_string);
    }

    // Sprites
    for(int i = 0; i < ani->sprite_count; i++) {
        if((ret = sd_sprite_save(w, ani->sprites[i])) != SD_SUCCESS) {
            return ret;
        }
    }

    return SD_SUCCESS;
}
