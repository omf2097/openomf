#include "animation.h"
#include "sprite.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdlib.h>
#include <assert.h>

sd_animation* sd_animation_create() {
    sd_animation *anim = (sd_animation*)malloc(sizeof(sd_animation));
    anim->overlay_table=NULL;
    return anim;
}

void sd_animation_delete(sd_animation *anim) {
    if (anim->overlay_table != NULL) {
        free(anim->overlay_table);
    }
    free(anim);
}

int sd_animation_load(sd_reader *r, sd_animation *ani) {
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
        ani->sprites[i] = sd_sprite_create();
        sd_sprite_load(r, ani->sprites[i]);
    }

    // Return success if reader is still ok
    return sd_reader_ok(r);
}

void sd_animation_save(sd_writer *writer, sd_animation *anim) {

}

