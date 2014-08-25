#include "animation_misc.h"
#include "conversions.h"
#include <string.h>
#include <stdio.h>

int sprite_key_get_id(const char* key) {
    if(strcmp(key, "x") == 0) return 0;
    if(strcmp(key, "y") == 0) return 1;
    if(strcmp(key, "index") == 0) return 2;
    if(strcmp(key, "missing") == 0) return 3;
    if(strcmp(key, "image") == 0) return 4;
    return -1;
}

void sprite_set_key(sd_sprite *s, const char **key, int kcount, const char *value) {
    switch(sprite_key_get_id(key[0])) {
        case 0: s->pos_x = conv_word(value); break;
        case 1: s->pos_y = conv_word(value); break;
        case 2: s->index = conv_ubyte(value); break;
        case 3: s->missing = conv_ubyte(value); break;
        case 4: printf("Value setting not supported for this key.\n"); break;
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
        case 4: printf("Value fetching not supported for this key.\n"); break;
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
    printf("* image\n");
}

void sprite_export_key(sd_sprite *s, const char **key, int kcount, const char *filename, sd_bk_file *bk) {
    switch(sprite_key_get_id(key[0])) {
        case 1:
        case 2:
        case 3:
            printf("Value fetching not supported for this key.\n");
            break;
        case 4: {
            sd_palette *pal = sd_bk_get_palette(bk, 0);
            if(pal == NULL) {
                printf("Palette required for exporting to PNG.\n");
                return;
            }
            sd_vga_image img;
            int ret = sd_sprite_vga_decode(&img, s);
            if(ret != SD_SUCCESS) {
                printf("Sprite decoding failed.\n");
                return;
            }
            ret = sd_vga_image_to_png(&img, pal, filename);
            sd_vga_image_free(&img);
            if(ret != SD_SUCCESS) {
                printf("Error while exporting sprite to %s: %s\n",
                    filename,
                    sd_get_error(ret));
                return;
            }
            }
            break;
        default:
            printf("Unknown key!\n");
    }
}

void sprite_import_key(sd_sprite *s, const char **key, int kcount, const char *filename, int transparent_index) {
    switch(sprite_key_get_id(key[0])) {
        case 1:
        case 2:
        case 3:
            printf("Value fetching not supported for this key.\n");
            break;
        case 4: {
            sd_vga_image img;
            int ret = sd_vga_image_from_png(&img, filename);
            if(ret != SD_SUCCESS) {
                printf("Error while importing sprite from %s: %s\n",
                    filename,
                    sd_get_error(ret));
                return;
            }

            ret = sd_vga_image_stencil_index(&img, transparent_index);
            if(ret != SD_SUCCESS) {
                printf("Stencil oculd not be set: %s\n", sd_get_error(ret));
                return;
            }

            ret = sd_sprite_vga_encode(s, &img);
            sd_vga_image_free(&img);
            if(ret != SD_SUCCESS) {
                printf("Error while converting VGA image to sprite.\n");
                return;
            }

            }
            break;
        default:
            printf("Unknown key!\n");
    }
}

void sprite_info(sd_sprite *s, int anim, int sprite) {
    printf("Animation #%d, Sprite #%d information:\n", anim, sprite);
    printf(" * X:        %d\n", s->pos_x);
    printf(" * Y:        %d\n", s->pos_y);
    printf(" * W:        %d\n", s->width);
    printf(" * H:        %d\n", s->height);
    printf(" * Index:    %d\n", s->index);
    printf(" * Missing:  %d\n", s->missing);
    printf(" * Length:   %d\n", s->len);
}

void anim_common_info(sd_animation *ani) {
    printf("Common animation header:\n");
    printf(" * Start X:          %d\n", ani->start_x);
    printf(" * Start Y:          %d\n", ani->start_y);
    printf(" * Animation header: %d\n", ani->null);
    printf(" * Collision coords: %d\n", ani->coord_count);
    for(int i = 0; i < ani->coord_count; i++) {
        printf("   - x,y = (%d,%d), null = %d, frame_id = %d\n",
                ani->coord_table[i].x, ani->coord_table[i].y,
                ani->coord_table[i].null, ani->coord_table[i].frame_id);
    }
    printf(" * Sprites:          %d\n", ani->sprite_count);
    printf(" * Animation str:    %s\n", ani->anim_string);
    printf(" * Extra strings:    %d\n", ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        printf("   - %s\n", ani->extra_strings[i]);
    }
}

int anim_key_get_id(const char* key) {
    if(strcmp(key, "ani_header") == 0) return 7;
    if(strcmp(key, "collision") == 0) return 8;
    if(strcmp(key, "anim_str") == 0) return 9;
    if(strcmp(key, "extra_str") == 0) return 11;
    if(strcmp(key, "start_x") == 0) return 12;
    if(strcmp(key, "start_y") == 0) return 13;
    return -1;
}

void anim_keylist() {
    printf("* start_x\n");
    printf("* start_y\n");
    printf("* ani_header\n");
    printf("* collision <collision #>\n");
    printf("* anim_str\n");
    printf("* extra_str <str #>\n");
}

void anim_push(sd_animation *ani) {
    sd_sprite sprite;
    sd_sprite_create(&sprite);
    sd_animation_push_sprite(ani, &sprite);
    printf("New sprite pushed to animation. Animation now has %d sprites.\n", ani->sprite_count);
}

void anim_pop(sd_animation *ani) {
    sd_animation_pop_sprite(ani);
    printf("Last sprite popped from animation. Animation now has %d sprites.\n", ani->sprite_count);
}

void anim_set_key(sd_animation *ani, int kn, const char **key, int kcount, const char *value) {
    int tmp = 0;
    switch(kn) {
        case 7:
            ani->null = conv_dword(value); break; 
        case 8:  
            /*if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->col_coord_count) {
                    ani->col_coord_table[tmp] = conv_udword(value);
                } else {
                    printf("Overlay index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                printf("Key overlay requires 1 parameter!\n");
                return;
            }*/
            printf("Coord value setting not supported yet!\n");
            break; 
        case 9:  sd_animation_set_anim_string(ani, value); break;
        case 11:  
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->extra_string_count) {
                    sd_animation_set_extra_string(ani, tmp, value);
                } else {
                    printf("Extra string table index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                printf("Key extra_str requires 1 parameter!\n");
                return;
            }
            break;
        case 12:  ani->start_x = conv_word(value); break;
        case 13:  ani->start_y = conv_word(value); break;
        default:
            printf("Unknown key!\n");
            return;
    }
    printf("Value set!\n");
}

void anim_get_key(sd_animation *ani, int kn, const char **key, int kcount, int pcount) {
    int tmp = 0;
    switch(kn) {
        case 7:
            printf("%d\n", ani->null);
            break; 
        case 8:
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->coord_count) {
                    printf("x,y = (%d,%d), null = %d, frame_id = %d\n",
                            ani->coord_table[tmp].x, 
                            ani->coord_table[tmp].y,
                            ani->coord_table[tmp].null, 
                            ani->coord_table[tmp].frame_id);
                } else {
                    printf("Collision table index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                for(int i = 0; i < ani->coord_count; i++) {
                    printf("x,y = (%d,%d), null = %d, frame_id = %d\n",
                            ani->coord_table[i].x, 
                            ani->coord_table[i].y,
                            ani->coord_table[i].null, 
                            ani->coord_table[i].frame_id);
                }
                printf("\n");
            }
            break;
        case 9:
            if (pcount > 0) {
                sd_stringparser *parser = sd_stringparser_create();
                int err = sd_stringparser_set_string(parser, ani->anim_string);
                if(err) {
                    printf("Animation string parser error: %s (%s)\n", sd_get_error(err), ani->anim_string);
                } else {
                    sd_stringparser_prettyprint(parser);
                }
                sd_stringparser_delete(parser);
            } else {
                printf("%s\n", ani->anim_string);
            }
            break;
        case 11: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->extra_string_count) {
                    if (pcount > 0) {
                        sd_stringparser *parser = sd_stringparser_create();
                        int err = sd_stringparser_set_string(parser, ani->extra_strings[tmp]);
                        if(err) {
                            printf("Animation string parser error: %s (%s)\n", sd_get_error(err), ani->extra_strings[tmp]);
                        } else {
                            sd_stringparser_prettyprint(parser);
                        }
                        sd_stringparser_delete(parser);
                    } else {
                        printf("%s\n", ani->extra_strings[tmp]);
                    }
                } else {
                    printf("Extra string table index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                for(int i = 0; i < ani->extra_string_count; i++) {
                    printf("%s ", ani->extra_strings[i]);
                }
                printf("\n");
            }
            break;
        case 12: printf("%d\n", ani->start_x); break;
        case 13: printf("%d\n", ani->start_y); break;
        default:
            printf("Unknown key!\n");
    }
}
