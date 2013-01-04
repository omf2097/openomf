#include "animation_misc.h"
#include "conversions.h"
#include <string.h>
#include <stdio.h>

int sprite_key_get_id(const char* key) {
    if(strcmp(key, "x") == 0) return 0;
    if(strcmp(key, "y") == 0) return 1;
    if(strcmp(key, "index") == 0) return 2;
    if(strcmp(key, "missing") == 0) return 3;
    return -1;
}

void sprite_set_key(sd_sprite *s, const char **key, int kcount, const char *value) {
    switch(sprite_key_get_id(key[0])) {
        case 0: s->pos_x = conv_word(value); break;
        case 1: s->pos_y = conv_word(value); break;
        case 2: s->index = conv_ubyte(value); break;
        case 3: s->missing = conv_ubyte(value); break;
        default:
            printf("Unknown key!\n");
            return;
    }
    printf("Value set!\n");
}

void sprite_get_key(sd_sprite *s, const char **key, int kcount) {
    switch(sprite_key_get_id(key[0])) {
        case 0: printf("%d\n", s->pos_x); break;
        case 1: printf("%d\n", s->pos_y); break;
        case 2: printf("%d\n", s->index); break;
        case 3: printf("%d\n", s->missing); break;
        default:
            printf("Unknown key!\n");
    }
}

void sprite_keylist() {
    printf("Valid field keys for Sprite structure:\n");
    printf("* x\n");
    printf("* y\n");
    printf("* index\n");
    printf("* missing\n");
}

void sprite_info(sd_sprite *s, int anim, int sprite) {
    printf("Animation #%d, Sprite #%d information:\n", anim, sprite);
    printf(" * X:        %d\n", s->pos_x);
    printf(" * Y:        %d\n", s->pos_y);
    printf(" * W:        %d\n", s->img->w);
    printf(" * H:        %d\n", s->img->h);
    printf(" * Index:    %d\n", s->index);
    printf(" * Missing:  %d\n", s->missing);
    printf(" * Length:   %d\n", s->img->len);
}

void anim_common_info(sd_animation *ani) {
    printf("Common animation header:\n");
    printf(" * Start X:         %d\n", ani->start_x);
    printf(" * Start Y:         %d\n", ani->start_y);
    printf(" * Animation header:  ");
    for(int i = 0; i < 4; i++) {
        printf("%d ", (uint8_t)ani->unknown_a[i]);
    }
    printf("\n");
    printf(" * Overlays:        %d\n", ani->overlay_count);
    for(int i = 0; i < ani->overlay_count; i++) {
        printf("   - %d\n", ani->overlay_table[i]);
    }
    printf(" * Sprites:         %d\n", ani->frame_count);
    printf(" * Animation str:   %s\n", ani->anim_string);
    printf(" * Unknown:         %d\n", ani->unknown_b);
    printf(" * Extra strings:   %d\n", ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        printf("   - %s\n", ani->extra_strings[i]);
    }
}
