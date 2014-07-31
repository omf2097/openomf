#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/helpers.h"
#include "shadowdive/colcoord.h"
#include "shadowdive/animation.h"
#include "shadowdive/sprite.h"
#include "shadowdive/error.h"
#include <stdlib.h>
#include <string.h>

int sd_animation_create(sd_animation *ani) {
    if(ani == NULL) {
        return SD_INVALID_INPUT;
    }

    ani->start_x = 0;
    ani->start_y = 0;
    ani->null = 0;
    ani->col_coord_count = 0;
    ani->sprite_count = 0;
    ani->extra_string_count = 0;
    
    for(int i = 0; i < SD_SPRITE_COLCOORD_COUNT_MAX; i++) {
        col_coord_create(ani->col_coord_table[i]);
    }
    for(int i = 0; i < SD_SPRITE_COUNT_MAX; i++) {
        ani->sprites[i] = NULL;
    }
    memset(ani->anim_string, 0, SD_ANIMATION_STRING_MAX);
    for(int i = 0; i < SD_SPRITE_EXTRASTR_COUNT_MAX; i++) {
        memset(ani->extra_strings[i], 0, SD_EXTRA_STRING_MAX);
    }
    return SD_SUCCESS;
}

int sd_animation_copy(sd_animation *ani, const sd_animation *src) {
    int ret;
    memcpy(ani, src, sizeof(sd_animation));

    // Copy sprites
    for(int i = 0; i < ani->sprite_count; i++) {
        sd_sprite *new = malloc(sizeof(sd_sprite));
        if(new == NULL) {
            return SD_OUT_OF_MEMORY;
        }
        ret = sd_sprite_copy(new, ani->sprites[i]);
        if(ret != SD_SUCCESS) {
            return ret;
        }
        ani->sprites[i] = new;
    }
    return SD_SUCCESS;
}

void sd_animation_free(sd_animation *anim) {
    for(int i = 0; i < anim->sprite_count; i++) {
        sd_sprite_free(anim->sprites[i]);
    }
}

int sd_animation_set_anim_string(sd_animation *ani, const char *str) {
    if(strlen(str) >= SD_ANIMATION_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(ani->anim_string, str);
    return SD_SUCCESS;
}

int sd_animation_set_extra_string(sd_animation *ani, int num, const char *str) {
    if(num < 0 || num >= SD_SPRITE_EXTRASTR_COUNT_MAX) {
        return SD_INVALID_INPUT;
    }
    if(strlen(str) >= SD_EXTRA_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(ani->extra_strings[num], str);
    return SD_SUCCESS;
}

int sd_animation_set_sprite(sd_animation *anim, int num, const sd_sprite *sprite) {
    if(num < 0 || num >= anim->sprite_count) {
        return SD_INVALID_INPUT;
    }
    if(anim->sprites[num] != NULL) {
        free(anim->sprites[num]);
    }
    anim->sprites[num] = malloc(sizeof(sd_sprite));
    if(anim->sprites[num] == NULL) {
        return SD_OUT_OF_MEMORY;
    }
    memcpy(anim->sprites[num], sprite, sizeof(sd_sprite));

    return SD_SUCCESS;
}

int sd_animation_push_sprite(sd_animation *anim, const sd_sprite *sprite) {

}

int sd_animation_pop_sprite(sd_animation *anim, sd_sprite *sprite) {

}

sd_sprite* sd_animation_get_sprite(sd_animation *anim, int num) {
    if(num < 0 || num >= SD_SPRITE_EXTRASTR_COUNT_MAX) {
        return NULL;
    }
    return anim->sprites[num];
}

int sd_animation_load(sd_reader *r, sd_animation *ani) {
    // Animation header
    ani->start_x = sd_read_word(r);
    ani->start_y = sd_read_word(r);
    sd_read_buf(r, ani->unknown_a, 4);
    ani->col_coord_count = sd_read_uword(r);
    ani->sprite_count = sd_read_ubyte(r);
    
    // Read collision point data
    ani->col_coord_table = (col_coord*)malloc(sizeof(col_coord)*ani->col_coord_count);
    uint32_t tmp;
    int32_t a,b;
    for(int i = 0; i < ani->col_coord_count; i++) {
        tmp = sd_read_udword(r);
        a = tmp & 0xffff;
        b = (tmp & 0xffff0000) >> 16;
        ani->col_coord_table[i].x = ((a & 0x3ff) << (6+16)) >> (6+16);
        ani->col_coord_table[i].x_ext = (a >> 10);
        ani->col_coord_table[i].y = ((b & 0x3ff) << (6+16)) >> (6+16);
        ani->col_coord_table[i].y_ext = (b >> 10);
    }

    // Animation string header
    uint16_t anim_string_len = sd_read_uword(r);
    sd_read_buf(r, ani->anim_string, anim_string_len);

    // Extra animation strings
    ani->extra_string_count = sd_read_ubyte(r);
    ani->extra_strings = (char**)malloc(sizeof(char*)*ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        uint16_t size = sd_read_uword(r);
        ani->extra_strings[i] = malloc(size+1);
        sd_read_buf(r, ani->extra_strings[i], size+1);
        if(ani->extra_strings[i][size] != 0) {
            return SD_FILE_PARSE_ERROR;
        }
    }

    // Sprites
    ani->sprites = (sd_sprite**)malloc(sizeof(sd_sprite*) * ani->frame_count);
    for(int i = 0; i < ani->frame_count; i++) {
        // finally, the actual sprite!
        ani->sprites[i] = sd_sprite_create();
        if(sd_sprite_load(r, ani->sprites[i])) {
            return SD_FILE_PARSE_ERROR;
        }
    }

    // Return success
    return SD_SUCCESS;
}

void sd_animation_save(sd_writer *writer, sd_animation *ani) {
    // Animation header
    sd_write_word(writer, ani->start_x);
    sd_write_word(writer, ani->start_y);
    sd_write_buf(writer, ani->unknown_a, 4);
    sd_write_uword(writer, ani->col_coord_count);
    sd_write_ubyte(writer, ani->frame_count);
    
    // collision table
    uint32_t tmp;
    for(int i = 0; i < ani->col_coord_count; i++) {
        tmp = (ani->col_coord_table[i].y_ext & 0x3f);
        tmp = tmp << 10;
        tmp = (tmp | (ani->col_coord_table[i].y & 0x3ff));
        tmp = tmp << 6;
        tmp = (tmp | (ani->col_coord_table[i].x_ext & 0x3f));
        tmp = tmp << 10;
        tmp = (tmp | (ani->col_coord_table[i].x & 0x3ff));
        sd_write_udword(writer, tmp);
    }

    // Animation string header
    uint16_t a_size = strlen(ani->anim_string);
    sd_write_uword(writer, a_size);
    sd_write_buf(writer, ani->anim_string, a_size);
    sd_write_ubyte(writer, 0);

    // Extra animation strings
    uint16_t s_size = 0;
    sd_write_ubyte(writer, ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        s_size = strlen(ani->extra_strings[i]);
        sd_write_uword(writer, s_size);
        sd_write_buf(writer, ani->extra_strings[i], s_size);
        sd_write_ubyte(writer, 0);
    }

    // Sprites
    for(int i = 0; i < ani->frame_count; i++) {
        sd_sprite_save(writer, ani->sprites[i]);
    }
}

