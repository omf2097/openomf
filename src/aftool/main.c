/** @file main.c
  * @brief .AF file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

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

int check_move_sprite(sd_af_file *af, int move, int sprite) {
    if(move > 70 || move < 0 || af->moves[move] == 0) {
        printf("animation #%d does not exist.\n", move);
        return 0;
    }
    if(sprite < 0 || af->moves[move]->animation->sprites[sprite] == 0 || sprite >= af->moves[move]->animation->frame_count) {
        printf("Sprite #%d does not exist.\n", sprite);
        return 0;
    }
    return 1;
}

int check_move(sd_af_file *af, int move) {
    if(af->moves[move] == 0 || move > 70 || move < 0) {
        printf("animation #%d does not exist.\n", move);
        return 0;
    }
    return 1;
}

int sprite_key_get_id(const char* key) {
    if(strcmp(key, "x") == 0) return 0;
    if(strcmp(key, "y") == 0) return 1;
    if(strcmp(key, "index") == 0) return 2;
    if(strcmp(key, "missing") == 0) return 3;
    return -1;
}

void sprite_set_key(sd_af_file *af, int move, int sprite, const char **key, int kcount, const char *value) {
    if(!check_move_sprite(af, move, sprite)) return;
    sd_sprite *s = af->moves[move]->animation->sprites[sprite];
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

void sprite_get_key(sd_af_file *af, int move, int sprite, const char **key, int kcount) {
    if(!check_move_sprite(af, move, sprite)) return;
    sd_sprite *s = af->moves[move]->animation->sprites[sprite];
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

void sprite_info(sd_af_file *af, int move, int sprite) {
    if(!check_move_sprite(af, move, sprite)) return;

    sd_sprite *s = af->moves[move]->animation->sprites[sprite];
    printf("animation #%d, Sprite #%d information:\n", move, sprite);
    printf(" * X:        %d\n", s->pos_x);
    printf(" * Y:        %d\n", s->pos_y);
    printf(" * W:        %d\n", s->img->w);
    printf(" * H:        %d\n", s->img->h);
    printf(" * Index:    %d\n", s->index);
    printf(" * Missing:  %d\n", s->missing);
    printf(" * Length:   %d\n", s->img->len);
}

int anim_key_get_id(const char* key) {
    if(strcmp(key, "ani_header") == 0) return 7;
    if(strcmp(key, "overlay") == 0) return 8;
    if(strcmp(key, "anim_str") == 0) return 9;
    if(strcmp(key, "unknown") == 0) return 10;
    if(strcmp(key, "extra_str") == 0) return 11;
    if(strcmp(key, "start_x") == 0) return 12;
    if(strcmp(key, "start_y") == 0) return 13;
    if(strcmp(key, "move_footer") == 0) return 14;
    if(strcmp(key, "move_string") == 0) return 15;
    if(strcmp(key, "footer_string") == 0) return 16;
    return -1;
}

void move_get_key(sd_af_file *af, int move, const char **key, int kcount) {
    int tmp = 0;
    if(!check_move(af, move)) return;
    sd_move *mv = af->moves[move];
    sd_animation *ani = mv->animation;
    switch(anim_key_get_id(key[0])) {
        case 7: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 4) {
                    printf("%d\n", ani->unknown_a[tmp]);
                } else {
                    printf("Header index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                for(int i = 0; i < 4; i++) {
                    printf("%d ", (uint8_t)ani->unknown_a[i]);
                }
                printf("\n");
            }
            break; 
        case 8:
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->overlay_count) {
                    printf("%d\n", ani->overlay_table[tmp]);
                } else {
                    printf("Overlay index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                for(int i = 0; i < ani->overlay_count; i++) {
                    printf("%d ", ani->overlay_table[i]);
                }
                printf("\n");
            }
            break;
        case 9: printf("%s\n", ani->anim_string); break;
        case 10: printf("%d\n", ani->unknown_b); break;
        case 11: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < ani->extra_string_count) {
                    printf("%s\n", ani->extra_strings[tmp]);
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
        case 14:
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 21) {
                    printf("%d\n", mv->unknown[tmp]);
                } else {
                    printf("Move string table index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                for(int i = 0; i < 21; i++) {
                    printf("%d ", (uint8_t)mv->unknown[i]);
                }
                printf("\n");
            }
            break;
        case 15: printf("%s\n", mv->move_string); break;
        case 16: printf("%s\n", mv->footer_string ? mv->footer_string : "(null)"); break;

        default:
            printf("Unknown key!\n");
    }
}


void move_keylist() {
    printf("Valid field keys for Move structure:\n");
    printf("* start_x\n");
    printf("* start_y\n");
    printf("* ani_header <byte #>\n");
    printf("* overlay <overlay #>\n");
    printf("* anim_str\n");
    printf("* unknown\n");
    printf("* extra_str <str #>\n");
    printf("* move_footer <byte #>\n");
    printf("* move_string\n");
    printf("* footer_string\n");
}

void move_info(sd_af_file *af, int move) {
    if(!check_move(af, move)) return;
    sd_move *mv = af->moves[move];
    sd_animation *ani = mv->animation;
    
    printf("Move #%d information:\n", move);
    
    printf("\nCommon animation header:\n");
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

    printf(" * Move footer:     ");
    for(int i = 0; i < 21; i++) {
        printf("%d ", (uint8_t)mv->unknown[i]);
    }
    printf("\n");

    printf(" * Move string:     %s\n", mv->move_string);
    printf(" * Footer string:   %s\n", mv->footer_string);
}

int af_key_get_id(const char* key) {
    if(strcmp(key, "fileid") == 0) return 0;
    if(strcmp(key, "unknown_a") == 0) return 1;
    if(strcmp(key, "endurance") == 0) return 2;
    if(strcmp(key, "unknown_b") == 0) return 3;
    if(strcmp(key, "power") == 0) return 4;
    if(strcmp(key, "forward_speed") == 0) return 5;
    if(strcmp(key, "reverse_speed") == 0) return 6;
    if(strcmp(key, "jump_speed") == 0) return 7;
    if(strcmp(key, "fall_speed") == 0) return 8;
    if(strcmp(key, "unknown_c") == 0) return 9;
    if(strcmp(key, "footer") == 0) return 10;
    return -1;
}


void af_get_key(sd_af_file *af, const char **key, int kcount) {
    int tmp = 0;
    switch(af_key_get_id(key[0])) {
        case 0: printf("%d\n", af->file_id); break;
        case 1: printf("%d\n", af->unknown_a); break;
        case 2: printf("%d\n", af->endurance); break;
        case 3: printf("%d\n", af->unknown_b); break;
        case 4: printf("%d\n", af->power); break;
        case 5: printf("%d\n", af->forward_speed); break;
        case 6: printf("%d\n", af->reverse_speed); break;
        case 7: printf("%d\n", af->jump_speed); break;
        case 8: printf("%d\n", af->fall_speed); break;
        case 9: printf("%d\n", af->unknown_c); break;
        case 10: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 30) {
                    printf("%d\n", af->footer[tmp]);
                } else {
                    printf("Footer index %d does not exist!\n", tmp);
                }
            } else {
                for(int i = 0; i < 30; i++) { printf("%d ", af->footer[i]); } printf("\n"); 
            }
            break;
        default:
            printf("Unknown key!\n");
    }
}

void af_keylist() {
    printf("Valid field keys for AF file root:\n");
    printf("* fileid\n");
    printf("* unknown_a\n");
    printf("* endurance\n");
    printf("* unknown_b\n");
    printf("* power\n");
    printf("* forward_speed\n");
    printf("* reverse_speed\n");
    printf("* jump_speed\n");
    printf("* fall_speed\n");
    printf("* unknown_c\n");
    printf("* footer <byte #>\n");
}


void af_info(sd_af_file *af) {
    printf("AF File information:\n");
    printf(" * File ID: %d\n", af->file_id);
    printf(" * Unknown A: %d\n", af->unknown_a);
    printf(" * Endurance: %d\n", af->endurance);
    printf(" * Unknown B: %d\n", af->unknown_b);
    printf(" * Power: %d\n", af->power);
    printf(" * Fwd speed: %d\n", af->forward_speed);
    printf(" * Rev speed: %d\n", af->reverse_speed);
    printf(" * Jump speed: %d\n", af->jump_speed);
    printf(" * Fall speed: %d\n", af->fall_speed);
    printf(" * Unknown C: %d\n", af->unknown_c);
    
    printf(" * animations:\n");
    for(int i = 0; i < 70; i++) {
        if(af->moves[i])
            printf("   - %d\n", i);
    }
    
    printf(" * Footer (hex): ");
    for(int k = 0; k < 30; k++) {
        printf("%d ", af->footer[k]);
    }
    printf("\n");
}



int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .AF file");
    struct arg_int *move = arg_int0("m", "move", "<move_id>", "Select move");
    struct arg_lit *all_moves = arg_lit0("A", "all_moves", "All moves");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --move)");
    struct arg_lit *keylist = arg_lit0(NULL, "keylist", "Prints a list of valid fields for --key.");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 2, "Select key");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .AF file");
    struct arg_file *palette = arg_file0("p", "palette", "<file>", "BK file for palette");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,move,all_moves,sprite,keylist,key,value,output,palette,end};
    const char* progname = "aftool";
    
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
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 .AF file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
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
    sd_af_file *af = sd_af_create();
    int ret = sd_af_load(af, file->filename[0]);
    if(ret) {
        printf("Unable to load AF file! Make sure the file exists and is a valid AF file.\n");
        goto exit_1;
    }
    
    // Argument dependencies
    if(move->count == 0) {
        if(sprite->count > 0) {
            printf("--sprite requires --move\n");
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


    if(sprite->count > 0) {
        if(key->count > 0) {
            /*if(value->count > 0) {*/
                /*sprite_set_key(af, move->ival[0], sprite->ival[0], key->sval, key->count, value->sval[0]);*/
            /*} else {*/
                sprite_get_key(af, move->ival[0], sprite->ival[0], key->sval, key->count);
            /*}*/
        } else if(keylist->count > 0) {
            sprite_keylist();
        /*} else if(play->count > 0) {*/
            /*sprite_play(af, _sc, move->ival[0], sprite->ival[0]);*/
        } else {
            sprite_info(af, move->ival[0], sprite->ival[0]);
        }
    } else if(move->count > 0) {
        if(key->count > 0) {
            /*if(value->count > 0) {*/
                /*move_set_key(af, move->ival[0], key->sval, key->count, value->sval[0]);*/
            /*} else {*/
                move_get_key(af, move->ival[0], key->sval, key->count);
            /*}*/
        } else if(keylist->count > 0) {
            move_keylist();
        /*} else if(play->count > 0) {*/
            /*move_play(af, _sc, move->ival[0]);*/
        } else {
            move_info(af, move->ival[0]);
        }
    } else if(all_moves->count > 0) {
        for(int i = 0; i < 70; i++) {
            if (af->moves[i]) {
                if(key->count > 0) {
                    /*if(value->count > 0) {*/
                        /*move_set_key(af, 1, key->sval, key->count, value->sval[0]);*/
                    /*} else {*/
                        printf("move %2u: ", i);
                        move_get_key(af, i, key->sval, key->count);
                    /*}*/
                } else {
                    printf("\n");
                    move_info(af, i);
                }
            }
        }
    } else {
        if(key->count > 0) {
            /*if(value->count > 0) {*/
                /*af_set_key(af, key->sval, key->count, value->sval[0]);*/
            /*} else {*/
                af_get_key(af, key->sval, key->count);
            /*}*/
        } else if(keylist->count > 0) {
            af_keylist();
        } else {
            af_info(af);
        }
    }

    // Write output file
    if(output->count > 0) {
        sd_af_save(af, output->filename[0]);
    }
    
    // Quit
exit_1:
    sd_af_delete(af);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
