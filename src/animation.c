#include <stdlib.h>
#include <string.h>

#include "shadowdive/internal/helpers.h"
#include "shadowdive/colcoord.h"
#include "shadowdive/animation.h"
#include "shadowdive/sprite.h"
#include "shadowdive/error.h"

int sd_animation_create(sd_animation *ani) {
    if(ani == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(ani, 0, sizeof(sd_animation));
    return SD_SUCCESS;
}

int sd_animation_copy(sd_animation *dst, const sd_animation *src) {
    int ret;
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear destination
    memset(dst, 0, sizeof(sd_animation));

    // Copy source
    dst->start_x = src->start_x;
    dst->start_y = src->start_y;
    dst->null = src->null;
    dst->sprite_count = src->sprite_count;
    dst->coord_count = src->coord_count;
    dst->extra_string_count = src->extra_string_count;

    // Copy extra strings
    memcpy(dst->extra_strings, src->extra_strings, sizeof(src->extra_strings));

    // Copy col coordinates
    memcpy(dst->coord_table, src->coord_table, sizeof(src->coord_table));

    // Copy sprites
    for(int i = 0; i < SD_SPRITE_COUNT_MAX; i++) {
        if(src->sprites[i] != NULL) {
            if((dst->sprites[i] = malloc(sizeof(sd_sprite))) == NULL) {
                return SD_OUT_OF_MEMORY;
            }
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
            free(anim->sprites[i]);
        }
    }
}

int sd_animation_set_anim_string(sd_animation *ani, const char *str) {
    if(strlen(str) >= SD_ANIMATION_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(ani->anim_string, str);
    return SD_SUCCESS;
}

int sd_animation_get_extra_string_count(sd_animation *anim) {
    return anim->extra_string_count;
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

int sd_animation_push_extra_string(sd_animation *anim, const char *str) {
    if(strlen(str) >= SD_EXTRA_STRING_MAX) {
        return SD_INVALID_INPUT;
    }
    strcpy(anim->extra_strings[anim->extra_string_count++], str);
    return SD_SUCCESS;
}

int sd_animation_pop_extra_string(sd_animation *anim) {
    if(anim->extra_string_count <= 0) {
        return SD_INVALID_INPUT;
    }
    anim->extra_string_count--;
    return SD_SUCCESS;
}

char* sd_animation_get_extra_string(sd_animation *anim, int num) {
    if(num < 0 || num >= anim->extra_string_count) {
        return NULL;
    }
    return anim->extra_strings[num];
}

int sd_animation_get_sprite_count(sd_animation *anim) {
    return anim->sprite_count;
}

int sd_animation_set_sprite(sd_animation *anim, int num, const sd_sprite *sprite) {
    int ret;
    if(num < 0 || num >= anim->sprite_count) {
        return SD_INVALID_INPUT;
    }
    if(anim->sprites[num] != NULL) {
        sd_sprite_free(anim->sprites[num]);
        free(anim->sprites[num]);
    }
    if((anim->sprites[num] = malloc(sizeof(sd_sprite))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
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
    if((anim->sprites[anim->sprite_count] = malloc(sizeof(sd_sprite))) == NULL) {
        return SD_OUT_OF_MEMORY;
    }
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
    free(anim->sprites[anim->sprite_count]);

    return SD_SUCCESS;
}

sd_sprite* sd_animation_get_sprite(sd_animation *anim, int num) {
    if(num < 0 || num >= SD_SPRITE_EXTRASTR_COUNT_MAX) {
        return NULL;
    }
    return anim->sprites[num];
}

int sd_animation_load(sd_reader *r, sd_animation *ani) {
    int ret;
    uint32_t tmp;
    int32_t a,b;
    uint16_t size;

    // Animation header
    ani->start_x = sd_read_word(r);
    ani->start_y = sd_read_word(r);
    ani->null = sd_read_udword(r);
    ani->coord_count = sd_read_uword(r);
    ani->sprite_count = sd_read_ubyte(r);
    
    // Read collision point data
    for(int i = 0; i < ani->coord_count; i++) {
        tmp = sd_read_udword(r);
        a = tmp & 0xffff;
        b = (tmp & 0xffff0000) >> 16;
        ani->coord_table[i].x = ((a & 0x3ff) << (6+16)) >> (6+16);
        ani->coord_table[i].null = (a >> 10);
        ani->coord_table[i].y = ((b & 0x3ff) << (6+16)) >> (6+16);
        ani->coord_table[i].frame_id = (b >> 10);
    }

    // Animation string header
    size = sd_read_uword(r);
    if(size > 0) {
        sd_read_buf(r, ani->anim_string, size+1);
    }
    if(ani->anim_string[size] != 0) {
        return SD_FILE_PARSE_ERROR;
    }

    // Extra animation strings
    ani->extra_string_count = sd_read_ubyte(r);
    for(int i = 0; i < ani->extra_string_count; i++) {
        size = sd_read_uword(r);
        if(size > 0) {
            sd_read_buf(r, ani->extra_strings[i], size+1);
            if(ani->extra_strings[i][size] != 0) {
                return SD_FILE_PARSE_ERROR;
            }
        }
    }

    // Sprites
    for(int i = 0; i < ani->sprite_count; i++) {
        if((ani->sprites[i] = malloc(sizeof(sd_sprite))) == NULL) {
            return SD_OUT_OF_MEMORY;
        }
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
    uint32_t tmp;
    uint16_t size;

    if(ani == NULL || w == NULL) {
        return SD_INVALID_INPUT;
    }

    // Animation header
    sd_write_word(w, ani->start_x);
    sd_write_word(w, ani->start_y);
    sd_write_udword(w, ani->null);
    sd_write_uword(w, ani->coord_count);
    sd_write_ubyte(w, ani->sprite_count);
    
    // collision table
    for(int i = 0; i < ani->coord_count; i++) {
        tmp = (ani->coord_table[i].frame_id & 0x3f);
        tmp = tmp << 10;
        tmp = (tmp | (ani->coord_table[i].y & 0x3ff));
        tmp = tmp << 6;
        tmp = (tmp | (ani->coord_table[i].null & 0x3f));
        tmp = tmp << 10;
        tmp = (tmp | (ani->coord_table[i].x & 0x3ff));
        sd_write_udword(w, tmp);
    }

    // Animation string header
    size = strlen(ani->anim_string);
    sd_write_uword(w, size);
    sd_write_buf(w, ani->anim_string, size);
    sd_write_ubyte(w, 0);

    // Extra animation strings
    sd_write_ubyte(w, ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        size = strlen(ani->extra_strings[i]);
        sd_write_uword(w, size);
        sd_write_buf(w, ani->extra_strings[i], size);
        sd_write_ubyte(w, 0);
    }

    // Sprites
    for(int i = 0; i < ani->sprite_count; i++) {
        if((ret = sd_sprite_save(w, ani->sprites[i])) != SD_SUCCESS) {
            return ret;
        }
    }

    return SD_SUCCESS;
}

