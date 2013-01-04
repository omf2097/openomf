#include "shadowdive/animation.h"
#include "shadowdive/sprite.h"
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#include "shadowdive/internal/helpers.h"
#include "shadowdive/error.h"
#include <stdlib.h>
#include <string.h>

sd_animation* sd_animation_create() {
    sd_animation *ani = (sd_animation*)malloc(sizeof(sd_animation));
    ani->overlay_table = NULL;
    ani->anim_string = NULL;
    ani->extra_strings = NULL;
    ani->sprites = NULL;
    return ani;
}

void sd_animation_delete(sd_animation *anim) {
    if(anim->overlay_table)
        free(anim->overlay_table);
    if(anim->anim_string)
        free(anim->anim_string);
    if(anim->extra_strings) {
        for(int i = 0; i < anim->extra_string_count; i++) {
            free(anim->extra_strings[i]);
        }
        free(anim->extra_strings);
    }
    if(anim->sprites) {
        for(int i = 0; i < anim->frame_count; i++) {
            sd_sprite_delete(anim->sprites[i]);
        }
        free(anim->sprites);
    }
    free(anim);
}

void sd_animation_set_anim_string(sd_animation *ani, const char *str) {
    alloc_or_realloc((void**)&ani->anim_string, sizeof(str)+1);
    strcpy(ani->anim_string, str);
}

void sd_animation_set_extra_string(sd_animation *ani, int num, const char *str) {
    if(num < 0 || num >= ani->extra_string_count) {
        return;
    }
    alloc_or_realloc((void**)&ani->extra_strings[num], strlen(str)+1);
    strcpy(ani->extra_strings[num], str);
}

int sd_animation_load(sd_reader *r, sd_animation *ani) {
    // Animation header
    sd_read_buf(r, ani->unknown_a, 8);
    ani->overlay_count = sd_read_uword(r);
    ani->frame_count = sd_read_ubyte(r);
    ani->overlay_table = (uint32_t*)malloc(sizeof(uint32_t)*ani->overlay_count);
    sd_read_buf(r, (char*)ani->overlay_table, sizeof(uint32_t)*ani->overlay_count);

    // Animation string header
    uint16_t anim_string_len = sd_read_uword(r);
    ani->anim_string = (char*)malloc(anim_string_len + 1);
    sd_read_buf(r, ani->anim_string, anim_string_len + 1);
    if(ani->anim_string[anim_string_len] != 0) {
        return SD_FILE_PARSE_ERROR;
    }

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
    sd_write_buf(writer, ani->unknown_a, 8);
    sd_write_uword(writer, ani->overlay_count);
    sd_write_ubyte(writer, ani->frame_count);
    sd_write_buf(writer, (char*)ani->overlay_table, ani->overlay_count * sizeof(uint32_t));

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

