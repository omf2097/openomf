/** @file main.c
  * @brief .BK file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>

int clamp(int value, long low, long high) {
    if(value > high) return high;
    if(value < low) return low;
    return value;
}

uint8_t  conv_ubyte(const char* data) { return clamp(atoi(data), 0, 0xFF); }
int8_t   conv_byte (const char* data) { return clamp(atoi(data), -0x80, 0x80); }
uint16_t conv_uword (const char* data) { return clamp(atoi(data), 0, 0xFFFF); }
int16_t  conv_word (const char* data) { return clamp(atoi(data), -0x7FFF, 0x7FFF); }
uint32_t conv_udword (const char* data) { return atoi(data); }
int32_t  conv_dword (const char* data) { return atoi(data); }

int check_anim_sprite(sd_bk_file *bk, int anim, int sprite) {
    if(anim > 50 || anim < 0 || bk->anims[anim] == 0) {
        printf("Animation #%d does not exist.\n", anim);
        return 0;
    }
    if(sprite < 0 || bk->anims[anim]->animation->sprites[sprite] == 0 || sprite >= bk->anims[anim]->animation->frame_count) {
        printf("Sprite #%d does not exist.\n", sprite);
        return 0;
    }
    return 1;
}

int check_anim(sd_bk_file *bk, int anim) {
    if(bk->anims[anim] == 0 || anim > 50 || anim < 0) {
        printf("Animation #%d does not exist.\n", anim);
        return 0;
    }
    return 1;
}

// Sprites --------------------------------------------------------------

int sprite_key_get_id(const char* key) {
    if(strcmp(key, "x") == 0) return 0;
    if(strcmp(key, "y") == 0) return 1;
    if(strcmp(key, "index") == 0) return 2;
    if(strcmp(key, "missing") == 0) return 3;
    return -1;
}

void sprite_set_key(sd_bk_file *bk, int anim, int sprite, const char *key, const char *value) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
    switch(sprite_key_get_id(key)) {
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

void sprite_get_key(sd_bk_file *bk, int anim, int sprite, const char *key) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
    switch(sprite_key_get_id(key)) {
        case 0: printf("%d\n", s->pos_x); break;
        case 1: printf("%d\n", s->pos_y); break;
        case 2: printf("%d\n", s->index); break;
        case 3: printf("%d\n", s->missing); break;
        default:
            printf("Unknown key!\n");
    }
}

void sprite_play(sd_bk_file *bk, int anim, int sprite) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];

}

void sprite_keylist() {
    printf("Valid field keys for Sprite structure:\n");
    printf("* x\n");
    printf("* y\n");
    printf("* index\n");
    printf("* missing\n");
}

void sprite_info(sd_bk_file *bk, int anim, int sprite) {
    if(!check_anim_sprite(bk, anim, sprite)) return;

    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
    printf("Animation #%d, Sprite #%d information:\n", anim, sprite);
    printf(" * X:        %d\n", s->pos_x);
    printf(" * Y:        %d\n", s->pos_y);
    printf(" * W:        %d\n", s->img->w);
    printf(" * H:        %d\n", s->img->h);
    printf(" * Index:    %d\n", s->index);
    printf(" * Missing:  %d\n", s->missing);
    printf(" * Length:   %d\n", s->img->len);
}

// Animations --------------------------------------------------------------

int anim_key_get_id(const char* key) {
    if(strcmp(key, "null") == 0) return 0;
    if(strcmp(key, "chain_hit") == 0) return 1;
    if(strcmp(key, "chain_no_hit") == 0) return 2;
    if(strcmp(key, "repeat") == 0) return 3;
    if(strcmp(key, "probability") == 0) return 4;
    if(strcmp(key, "hazard_damage") == 0) return 5;
    if(strcmp(key, "bk_str") == 0) return 6;
    if(strcmp(key, "ani_header") == 0) return 7;
    if(strcmp(key, "overlay") == 0) return 8;
    if(strcmp(key, "anim_str") == 0) return 9;
    if(strcmp(key, "unknown") == 0) return 10;
    if(strcmp(key, "extra_str") == 0) return 11;
    return -1;
}

void anim_set_key(sd_bk_file *bk, int anim, const char *key, const char *value) {
    if(!check_anim(bk, anim)) return;
    switch(anim_key_get_id(key)) {
        case 0:  break;
        case 1:  break;
        case 2:  break;
        case 3:  break;
        case 4:  break;
        case 5:  break;
        case 6:  break;
        case 7:  break;
        case 8:  break;
        case 9:  break;
        case 10:  break;
        case 11:  break;
        default:
            printf("Unknown key!\n");
            return;
    }
    printf("Value set!\n");
}

void anim_get_key(sd_bk_file *bk, int anim, const char *key) {
    if(!check_anim(bk, anim)) return;
    switch(anim_key_get_id(key)) {
        case 0: printf("\n"); break;
        case 1: printf("\n"); break;
        case 2: printf("\n"); break;
        case 3: printf("\n"); break;
        case 4: printf("\n"); break;
        case 5: printf("\n"); break;
        case 6: printf("\n"); break;
        case 7: printf("\n"); break;
        case 8: printf("\n"); break;
        case 9: printf("\n"); break;
        case 10: printf("\n"); break;
        case 11: printf("\n"); break;
        default:
            printf("Unknown key!\n");
    }
}

void anim_play(sd_bk_file *bk, int anim) {
    if(!check_anim(bk, anim)) return;
    
}

void anim_keylist() {
    printf("Valid field keys for Animation structure:\n");
    printf("* null\n");
    printf("* chain_hit\n");
    printf("* chain_no_hit\n");
    printf("* repeat\n");
    printf("* probability\n");
    printf("* hazard_damage\n");
    printf("* bk_str\n");
    printf("* ani_header:<byte #>\n");
    printf("* overlay:<overlay #>\n");
    printf("* anim_str\n");
    printf("* unknown\n");
    printf("* extra_str:<str #>\n");
}

void anim_info(sd_bk_file *bk, int anim) {
    if(!check_anim(bk, anim)) return;
    sd_bk_anim *bka = bk->anims[anim];
    sd_animation *ani = bk->anims[anim]->animation;
    
    printf("Animation #%d information:\n", anim);
    
    printf("\nBK specific header:\n");
    printf(" * Null:            %d\n", bka->null);
    printf(" * Chain # if hit:  %d\n", bka->chain_hit);
    printf(" * Chain # not hit: %d\n", bka->chain_no_hit);
    printf(" * Repeat:          %d\n", bka->repeat);
    printf(" * Probability:     %d\n", bka->probability);
    printf(" * hazard damage:   %d\n", bka->hazard_damage);
    printf(" * String:          ");
    for(int i = 0; i < bka->unknown_size; i++) {
        printf("%c", bka->unknown_data[i]);
    }
    printf("\n");
    
    printf("\nCommon animation header:\n");
    printf(" * Unknown header:  ");
    for(int i = 0; i < 8; i++) {
        printf("%x ", (uint8_t)ani->unknown_a[i]);
    }
    printf("\n");
    printf(" * Overlays:        %d\n", ani->overlay_count);
    for(int i = 0; i < ani->overlay_count; i++) {
        printf("   - %d\n", ani->overlay_table[i]);
    }
    printf(" * Sprites:         %d\n", ani->frame_count);
    printf(" * Animation str:   %s\n", ani->anim_string);
    printf(" * Unknown B:       %d\n", ani->unknown_b);
    printf(" * Extra strings:   %d\n", ani->extra_string_count);
    for(int i = 0; i < ani->extra_string_count; i++) {
        printf("   - %s\n", ani->extra_strings[i]);
    }
    
}

// BK Root  --------------------------------------------------------------

int bk_key_get_id(const char* key) {
    if(strcmp(key, "fileid") == 0) return 0;
    if(strcmp(key, "palette") == 0) return 1;
    if(strcmp(key, "unknown") == 0) return 2;
    if(strcmp(key, "footer") == 0) return 3;
    return -1;
}

void bk_set_key(sd_bk_file *bk, const char *key, const char *value) {
    printf("Value (str): %s\n", value);
    printf("Value (int): %d\n", atoi(value));
    switch(bk_key_get_id(key)) {
        case 0: bk->file_id = conv_udword(value); break;
        case 1: break;
        case 2: bk->unknown_a = conv_ubyte(value); break;
        case 3: break;
        default:
            printf("Unknown key!\n");
            return;
    }
    printf("Value set!\n");
}

void bk_get_key(sd_bk_file *bk, const char *key) {
    switch(bk_key_get_id(key)) {
        case 0: printf("%d\n", bk->file_id); break;
        case 1: printf("Not implemented!"); break;
        case 2: printf("%d\n", bk->unknown_a); break;
        case 3: for(int i = 0; i < 30; i++) { printf("%x ", bk->footer[i]); } printf("\n"); break;
        default:
            printf("Unknown key!\n");
    }
}

void bk_keylist() {
    printf("Valid field keys for BK file root:\n");
    printf("* fileid\n");
    printf("* palette:<palette #>\n");
    printf("* unknown\n");
    printf("* footer:<byte #>\n");
}

void bk_info(sd_bk_file *bk) {
    printf("BK File information:\n");
    printf(" * File ID: %d\n", bk->file_id);
    printf(" * Palettes: %d\n", bk->num_palettes);
    printf(" * Unknown A: %d\n", bk->unknown_a);
    
    printf(" * Animations:\n");
    for(int i = 0; i < 50; i++) {
        if(bk->anims[i])
            printf("   - %d\n", i);
    }
    
    printf(" * Footer (hex): ");
    for(int k = 0; k < 30; k++) {
        printf("%x ", bk->footer[k]);
    }
    printf("\n");
}

// Main --------------------------------------------------------------

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .BK file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .BK file");
    struct arg_int *anim = arg_int0("a", "anim", "<animation_id>", "Select animation");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --anim)");
    struct arg_lit *keylist = arg_lit0(NULL, "keylist", "Prints a list of valid fields for --key.");
    struct arg_str *key = arg_str0(NULL, "key", "<key>", "Select key");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_str *play = arg_str0(NULL, "play", "<id>", "Play animation or sprite (requires --anim)");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,anim,sprite,keylist,key,value,play,end};
    const char* progname = "bktool";
    
    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }
    
    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-30s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 .BK file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
    }
    
    // Argument dependencies
    if(anim->count == 0) {
        if(sprite->count > 0) {
            printf("--sprite requires --anim\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
        if(play->count > 0) {
            printf("--play requires --anim\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
    }
    if(key->count == 0) {
        if(value->count > 0) {
            printf("--value requires --key\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
    }
    
    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }
    
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    
    // Load file
    sd_bk_file *bk = sd_bk_create();
    int ret = sd_bk_load(bk, file->filename[0]);
    if(ret) {
        printf("Unable to load BK file! Make sure the file exists and is a valid BK file.\n");
        goto exit_1;
    }
    
    // Handle args
    if(sprite->count > 0) {
        if(key->count > 0) {
            if(value->count > 0) {
                sprite_set_key(bk, anim->ival[0], sprite->ival[0], key->sval[0], value->sval[0]);
            } else {
                sprite_get_key(bk, anim->ival[0], sprite->ival[0], key->sval[0]);
            }
        } else if(keylist->count > 0) {
            sprite_keylist();
        } else if(play->count > 0) {
            sprite_play(bk, anim->ival[0], sprite->ival[0]);
        } else {
            sprite_info(bk, anim->ival[0], sprite->ival[0]);
        }
    } else if(anim->count > 0) {
        if(key->count > 0) {
            if(value->count > 0) {
                anim_set_key(bk, anim->ival[0], key->sval[0], value->sval[0]);
            } else {
                anim_get_key(bk, anim->ival[0], key->sval[0]);
            }
        } else if(keylist->count > 0) {
            anim_keylist();
        } else if(play->count > 0) {
            anim_play(bk, anim->ival[0]);
        } else {
            anim_info(bk, anim->ival[0]);
        }
    } else {
        if(key->count > 0) {
            if(value->count > 0) {
                bk_set_key(bk, key->sval[0], value->sval[0]);
            } else {
                bk_get_key(bk, key->sval[0]);
            }
        } else if(keylist->count > 0) {
            bk_keylist();
        } else {
            bk_info(bk);
        }
    }
    
    // Write output file
    if(output->count > 0) {
        sd_bk_save(bk, output->filename[0]);
    }
    
    // Quit
exit_1:
    sd_bk_delete(bk);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}