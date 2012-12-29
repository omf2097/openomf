#include "af.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include "animation.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

sd_af_file* sd_af_load(const char *filename) {
    // Initialize reader
    sd_reader *r = sd_reader_open(filename);
    if(!r) {
        return 0;
    }

    // Allocate structure
    sd_af_file *af = malloc(sizeof(sd_af_file));

    // Header
    af->file_id = sd_read_uword(r);
    af->unknown_a = sd_read_uword(r);
    af->endurance = sd_read_udword(r);
    sd_skip(r, 1);// TODO: Find out what this is
    af->power = sd_read_uword(r);
    af->forward_speed = sd_read_dword(r);
    af->reverse_speed = sd_read_dword(r);
    af->jump_speed = sd_read_dword(r);
    af->fall_speed = sd_read_dword(r);
    sd_skip(r, 2);// TODO: Find out what this is
    memset(af->moves, 0, sizeof(af->moves));

    // Read animations
    uint8_t moveno = 0;
    sd_move *move;
    sd_animation *ani;
    while(1) {
        moveno = sd_read_ubyte(r);
        printf("move number %d\n", moveno);
        if(moveno >= 70 || !sd_reader_ok(r)) {
            break;
        }

        // af Specific animation header
        // Initialize animation
        move = sd_move_create();
        ani = &move->animation;
        af->moves[moveno] = move;

        // Animation header
        sd_read_buf(r, ani->unknown_a, 8);
        ani->overlay_count = sd_read_uword(r);
        ani->frame_count = sd_read_ubyte(r);
        ani->overlay_table = (uint32_t*)malloc(sizeof(uint32_t)*ani->overlay_count);
        sd_read_buf(r, (char*)ani->overlay_table, sizeof(uint32_t)*ani->overlay_count);

        // Animation string header
        ani->anim_string_len = sd_read_uword(r);
        ani->anim_string = (char*)malloc(ani->anim_string_len + 1);
        sd_read_buf(r, ani->anim_string, ani->anim_string_len+1); // assume its null terminated
        printf("animation string %s\n", ani->anim_string);
        assert(ani->anim_string[ani->anim_string_len] == '\0');

        // Extra animation strings
        ani->extra_string_count = sd_read_ubyte(r);
        ani->extra_strings = (char**)malloc(sizeof(char*)*ani->extra_string_count);
        for(int i = 0; i < ani->extra_string_count; i++) {
            uint16_t size = sd_read_uword(r);
            ani->extra_strings[i] = malloc(size+1);
            // assume its null terminated
            sd_read_buf(r, ani->extra_strings[i], size+1);
            assert(ani->extra_strings[i][size] == '\0');
        }

        // Sprites
        ani->sprites = (sd_sprite**)malloc(sizeof(sd_sprite*) * ani->frame_count);
        for(int i = 0; i < ani->frame_count; i++) {
            // finally, the actual sprite!
            sd_sprite *sprite = sd_sprite_create();
            ani->sprites[i] = sprite;
            uint16_t len = sd_read_uword(r);
            sprite->pos_x = sd_read_word(r);
            sprite->pos_y = sd_read_word(r);
            uint16_t width = sd_read_uword(r);
            uint16_t height = sd_read_uword(r);
            sprite->index = sd_read_ubyte(r);
            sprite->missing = sd_read_ubyte(r);
            if (sprite->missing == 0) {
                // sprite data follows
                sprite->img = sd_sprite_image_create(width, height, len);
                sd_read_buf(r, sprite->img->data, len);
            } else {
                // TODO set the pointer to be the actual sprite, from the other animation, maybe?
                sprite->img = NULL;
            }
        }

        // Move footer
        sd_read_buf(r, move->unknown, 21);
        sd_read_buf(r, move->move_string, 21);
        int len = sd_read_uword(r);
        move->footer_string = (char*)malloc(len+1);
        sd_read_buf(r, move->footer_string, len);
        move->footer_string[len] = '0';
        printf("footer string %s\n", move->footer_string);
    }

    // Read footer
    sd_read_buf(r, af->footer, 30);

    // Close & return
    sd_reader_close(r);
    return af;
}

int sd_af_save(const char* filename, sd_af_file *af) {
    return 0;
}

void sd_af_delete(sd_af_file *af) {
    free(af);
}
