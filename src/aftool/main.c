/** @file main.c
  * @brief .AF file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include "../shared/animation_misc.h"
#include "../shared/conversions.h"

void move_info(sd_move *mv, sd_animation *ani, int move);

int check_move_sprite(sd_af_file *af, int move, int sprite) {
    if(move > 70 || move < 0 || af->moves[move] == 0) {
        printf("animation #%d does not exist.\n", move);
        return 0;
    }
    if(sprite < 0 || af->moves[move]->animation->sprites[sprite] == 0 || sprite >= af->moves[move]->animation->sprite_count) {
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

// Sprite stuff --------------------------------------

void sprite_play(sd_af_file *af, sd_bk_file *bk, int scale, int anim, int sprite) {
    if(!check_move_sprite(af, anim, sprite)) return;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Texture *rendertarget;
    SDL_Rect rect;
    SDL_Rect dstrect;
    sd_sprite *s = af->moves[anim]->animation->sprites[sprite];
    SDL_Window *window = SDL_CreateWindow(
            "OMF2097 Remake",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            320 * scale,
            200 * scale,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
            );

    if (!window) {
        printf("Could not create window: %s\n", SDL_GetError());
        return;
    }
    
    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", s->pos_x, s->pos_y, s->width, s->height, s->len);
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    uint32_t rmask, gmask, bmask, amask;

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    
    if((rendertarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 200)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    sd_rgba_image img;
    sd_sprite_rgba_decode(&img, s, bk->palettes[0], -1);

    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img.data, img.w, img.h, 32, img.w*4,
            rmask, gmask, bmask, amask))) {
        printf("Could not create surface: %s\n", SDL_GetError());
        return;
    }

    if ((texture = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    SDL_FreeSurface(surface);
    sd_rgba_image_free(&img);

    rect.x = s->pos_x + 160;
    rect.y = s->pos_y + 100;
    rect.w = s->width;
    rect.h = s->height;
    
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 320 * scale;
    dstrect.h = 200 * scale;

    while(1) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                break;
            } else if (e.type == SDL_KEYUP) {
                int i = anim;
                int changed = 0;
                switch (e.key.keysym.sym) {
                    case SDLK_RIGHT:
                        sprite = (sprite+1) % af->moves[anim]->animation->sprite_count;
                        printf("sprite is now %d\n", sprite);
                        changed = 1;
                        break;
                    case SDLK_LEFT:
                        sprite--;
                        if (sprite < 0) {
                            sprite = af->moves[anim]->animation->sprite_count - 1;
                        }
                        changed = 1;
                        break;
                    case SDLK_UP:
                        i++;
                        while (!check_move(af, i) && i < 50) {
                            i++;
                        }
                        if (i == 50) {
                            printf("no more animations\n");
                        } else {
                            anim = i;
                            printf("UP: animation is now %d\n", anim);
                            sprite = 0;
                        }
                        changed = 1;
                        break;
                    case SDLK_DOWN:
                        i--;
                        while (!check_move(af, i) && i >= 0) {
                            i--;
                        }
                        if (i < 0) {
                            printf("no previous animations\n");
                        } else {
                            anim = i;
                            printf("DOWN: animation is now %d\n", anim);
                            sprite = 0;
                        }
                        changed = 1;
                        break;
                    case SDLK_p:
                        {
                            // print the move info
                            sd_move *mv = af->moves[i];
                            sd_animation *ani = mv->animation;
                            move_info(mv, ani, i);
                            changed = 0;
                        }
                        break;
                    default:
                        changed = 0;
                }
                if (changed) {
                    s = af->moves[anim]->animation->sprites[sprite];
                    sd_sprite_rgba_decode(&img, s, bk->palettes[0], -1);
                    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", s->pos_x, s->pos_y, s->width, s->height, s->len);

                    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img.data, img.w, img.h, 32, img.w*4,
                                    rmask, gmask, bmask, amask))) {
                        printf("Could not create surface: %s\n", SDL_GetError());
                        return;
                    }

                    if ((texture = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
                        printf("Could not create texture: %s\n", SDL_GetError());
                        return;
                    }

                    SDL_FreeSurface(surface);
                    sd_rgba_image_free(&img);

                    rect.x = s->pos_x + 160;
                    rect.y = s->pos_y + 100;
                    rect.w = s->width;
                    rect.h = s->height;
                }
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, rendertarget);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderFillRect(renderer, &dstrect);
        SDL_RenderCopy(renderer, texture, NULL, &rect);

        // render the collision data
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for(int i = 0; i < af->moves[anim]->animation->coord_count; i++) {
            int x = af->moves[anim]->animation->coord_table[i].x;
            int y = af->moves[anim]->animation->coord_table[i].y;
            int frame_id = af->moves[anim]->animation->coord_table[i].frame_id;
            if(frame_id == sprite) {
                SDL_RenderDrawPoint(renderer, 160+x, 100+y);
            }
        }
        
        // Switch to screen target & scale
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, rendertarget, NULL, &dstrect);
        SDL_RenderPresent(renderer);
        SDL_Delay(1); // don't chew too much CPU
    }

    // Close and destroy the window
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
}

// Move stuff --------------------------------------------------

int move_key_get_id(const char* key) {
    if(strcmp(key, "unknown_0") == 0) return 30;
    if(strcmp(key, "unknown_2") == 0) return 31;
    if(strcmp(key, "unknown_4") == 0) return 33;
    if(strcmp(key, "unknown_5") == 0) return 34;
    if(strcmp(key, "unknown_6") == 0) return 35;
    if(strcmp(key, "unknown_7") == 0) return 36;
    if(strcmp(key, "unknown_8") == 0) return 37;
    if(strcmp(key, "unknown_9") == 0) return 38;
    if(strcmp(key, "unknown_10") == 0) return 39;
    if(strcmp(key, "unknown_11") == 0) return 40;
    if(strcmp(key, "next_anim_id") == 0) return 41;
    if(strcmp(key, "category") == 0) return 42;
    if(strcmp(key, "unknown_14") == 0) return 43;
    if(strcmp(key, "scrap_amount") == 0) return 44;
    if(strcmp(key, "successor_id") == 0) return 45;
    if(strcmp(key, "damage_amount") == 0) return 46;
    if(strcmp(key, "unknown_18") == 0) return 47;
    if(strcmp(key, "unknown_19") == 0) return 48;
    if(strcmp(key, "points") == 0) return 49;

    if(strcmp(key, "move_string") == 0) return 15;
    if(strcmp(key, "footer_string") == 0) return 16;
    return anim_key_get_id(key);
}

void move_set_key(sd_move *move, sd_animation *ani, const char **key, int kcount, const char *value) {
    int tmp = 0;
    int kn = move_key_get_id(key[0]);
    switch(kn) {

        case 30: move->unknown_0 = conv_uword(value); break;
        case 31: move->unknown_2 = conv_uword(value); break;
        case 33: move->unknown_4 = conv_ubyte(value); break;
        case 34: move->unknown_5 = conv_ubyte(value); break;
        case 35: move->unknown_6 = conv_ubyte(value); break;
        case 36: move->unknown_7 = conv_ubyte(value); break;
        case 37: move->unknown_8 = conv_ubyte(value); break;
        case 38: move->unknown_9 = conv_ubyte(value); break;
        case 39: move->unknown_10 = conv_ubyte(value); break;
        case 40: move->unknown_11 = conv_ubyte(value); break;
        case 41: move->next_anim_id = conv_ubyte(value); break;
        case 42: move->category = conv_ubyte(value); break;
        case 43: move->unknown_14 = conv_ubyte(value); break;
        case 44: move->scrap_amount = conv_ubyte(value); break;
        case 45: move->successor_id = conv_ubyte(value); break;
        case 46: move->damage_amount = conv_ubyte(value); break;
        case 47: move->unknown_18 = conv_ubyte(value); break;
        case 48: move->unknown_19 = conv_ubyte(value); break;
        case 49: move->points = conv_ubyte(value); break;

        case 15: 
            tmp = strlen(value)+1;
            if(tmp < 21) {
                memcpy(move->move_string, value, tmp);
            } else {
                printf("String is too long! (%u bytes) Maximum size for move_string is 21 characters!\n", tmp);
                return;
            }
            break;
        case 16: 
            sd_move_set_footer_string(move, value);
            break;
        default:
            anim_set_key(ani, kn, key, kcount, value);
            return;
    }
    printf("Value set!\n");
}

void move_get_key(sd_move *move, sd_animation *ani, const char **key, int kcount, int pcount) {
    int kn = move_key_get_id(key[0]);
    switch(kn) {
        case 30: printf("%d\n", move->unknown_0);
        case 31: printf("%d\n", move->unknown_2);
        case 33: printf("%d\n", move->unknown_4);
        case 34: printf("%d\n", move->unknown_5);
        case 35: printf("%d\n", move->unknown_6);
        case 36: printf("%d\n", move->unknown_7);
        case 37: printf("%d\n", move->unknown_8);
        case 38: printf("%d\n", move->unknown_9);
        case 39: printf("%d\n", move->unknown_10);
        case 40: printf("%d\n", move->unknown_11);
        case 41: printf("%d\n", move->next_anim_id);
        case 42: printf("%d\n", move->category);
        case 43: printf("%d\n", move->unknown_14);
        case 44: printf("%d\n", move->scrap_amount);
        case 45: printf("%d\n", move->successor_id);
        case 46: printf("%d\n", move->damage_amount);
        case 47: printf("%d\n", move->unknown_18);
        case 48: printf("%d\n", move->unknown_19);
        case 49: printf("%d\n", move->points);

        case 15:
            printf("%s\n", move->move_string); break;
        case 16:
            if (pcount > 0 && move->footer_string) {
                sd_stringparser *parser = sd_stringparser_create();
                int err = sd_stringparser_set_string(parser, ani->anim_string);
                if(err) {
                    printf("Animation string parser error: %s (%s)\n", sd_get_error(err), ani->anim_string);
                } else {
                    sd_stringparser_prettyprint(parser);
                }
                sd_stringparser_delete(parser);
            } else {
                printf("%s\n", move->footer_string ? move->footer_string : "(null)");
            }
            break;

        default:
            anim_get_key(ani, kn, key, kcount, pcount);
    }
}

void move_play(sd_af_file *af, sd_bk_file *bk, int scale, int anim) {
    sprite_play(af, bk, scale, anim, 0);
}

void move_keylist() {
    printf("Valid field keys for Move structure:\n");
    anim_keylist();

    printf("* unknown_0\n");
    printf("* unknown_2\n");
    printf("* unknown_4\n");
    printf("* unknown_5\n");
    printf("* unknown_6\n");
    printf("* unknown_7\n");
    printf("* unknown_8\n");
    printf("* unknown_9\n");
    printf("* unknown_10\n");
    printf("* unknown_11\n");
    printf("* next_anim_id\n");
    printf("* category\n");
    printf("* unknown_14\n");
    printf("* scrap_amount\n");
    printf("* successor_id\n");
    printf("* damage_amount\n");
    printf("* unknown_18\n");
    printf("* unknown_19\n");
    printf("* points\n");
    printf("* move_string <21 chars max>\n");
    printf("* footer_string\n");
}

void move_info(sd_move *move, sd_animation *ani, int move_id) {
    printf("Move #%d information:\n\n", move_id);
    
    anim_common_info(ani);

    printf("\nAF specific footer:\n");
    printf(" * unknown_0:       %d\n", move->unknown_0);
    printf(" * unknown_2:       %d\n", move->unknown_2);
    printf(" * unknown_4:       %d\n", move->unknown_4);
    printf(" * unknown_5:       %d\n", move->unknown_5);
    printf(" * unknown_6:       %d\n", move->unknown_6);
    printf(" * unknown_7:       %d\n", move->unknown_7);
    printf(" * unknown_8:       %d\n", move->unknown_8);
    printf(" * unknown_9:       %d\n", move->unknown_9);
    printf(" * unknown_10:      %d\n", move->unknown_10);
    printf(" * unknown_11:      %d\n", move->unknown_11);
    printf(" * next_anim_id:    %d\n", move->next_anim_id);
    printf(" * category:        %d\n", move->category);
    printf(" * unknown_14:      %d\n", move->unknown_14);
    printf(" * scrap_amount:    %d\n", move->scrap_amount);
    printf(" * successor_id:    %d\n", move->successor_id);
    printf(" * damage_amount:   %d\n", move->damage_amount);
    printf(" * unknown_18:      %d\n", move->unknown_18);
    printf(" * unknown_19:      %d\n", move->unknown_19);
    printf(" * points:          %d\n", move->points);

    printf(" * Move string:     %s\n", move->move_string);
    printf(" * Footer string:   %s\n", move->footer_string);
}

// AF Specific stuff -----------------------------------------------

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
    if(strcmp(key, "soundtable") == 0) return 10;
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
                    printf("%d\n", af->soundtable[tmp]);
                } else {
                    printf("Soundtable index %d does not exist!\n", tmp);
                }
            } else {
                for(int i = 0; i < 30; i++) { printf("%d ", af->soundtable[i]); } printf("\n"); 
            }
            break;
        default:
            printf("Unknown key!\n");
    }
}

void af_set_key(sd_af_file *af, const char **key, int kcount, const char *value) {
    int tmp = 0;
    switch(af_key_get_id(key[0])) {
        case 0: af->file_id = conv_uword(value); break;
        case 1: af->unknown_a = conv_uword(value); break;
        case 2: af->endurance = conv_udword(value); break;
        case 3: af->unknown_b = conv_ubyte(value); break;
        case 4: af->power = conv_uword(value); break;
        case 5: af->forward_speed = conv_dword(value); break;
        case 6: af->reverse_speed = conv_dword(value); break;
        case 7: af->jump_speed = conv_dword(value); break;
        case 8: af->fall_speed = conv_dword(value); break;
        case 9: af->unknown_c = conv_uword(value); break;
        case 10: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 30) {
                    af->soundtable[tmp] = conv_ubyte(value);
                } else {
                    printf("Soundtable index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                printf("Soundtable value requires index parameter (eg. --key soundtable --key 3).\n");
                return;
            }
        break;
        default:
            printf("Unknown key!\n");
            return;
    }
    printf("Value set!\n");
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
    printf(" * File ID:     %d\n", af->file_id);
    printf(" * Unknown A:   %d\n", af->unknown_a);
    printf(" * Endurance:   %d\n", af->endurance);
    printf(" * Unknown B:   %d\n", af->unknown_b);
    printf(" * Power:       %d\n", af->power);
    printf(" * Fwd speed:   %d\n", af->forward_speed);
    printf(" * Rev speed:   %d\n", af->reverse_speed);
    printf(" * Jump speed:  %d\n", af->jump_speed);
    printf(" * Fall speed:  %d\n", af->fall_speed);
    printf(" * Unknown C:   %d\n", af->unknown_c);
    
    printf(" * Animations:  ");
    int start = -1, last = -1;
    unsigned int m;
    for(m = 0; m < 70; m++) {
        if(af->moves[m]) {
            if(start == -1) {
                start = m;
                last = m;
            }
            if(m > last+1) {
                if(start == last) {
                    printf("%d, ", last);
                } else {
                    printf("%d-%d, ", start, last);
                }
                start = m;
            }
            last = m;
        }
    }
    if(m != last) {
        if(start == last) {
            printf("%d\n", last);
        } else {
            printf("%d-%d\n", start, last);
        }
    } else {
        printf("\n");
    }
    
    printf(" * Sound table:\n");
    printf("   |");
    for(int k = 0; k < 30; k++) {
        printf("%2d ", k);
    }
    printf("|\n");
    printf("   |");
    for(int k = 0; k < 30*3; k++) {
        printf("-");
    }
    printf("|\n");
    printf("   |");
    for(int k = 0; k < 30; k++) {
        printf("%2d ", af->soundtable[k]);
    }
    printf("|\n");
}



int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "Input .AF file");
    struct arg_lit *new = arg_lit0("n", "new", "Creates a new structure for editing.");
    struct arg_int *move = arg_int0("m", "move", "<move_id>", "Select move");
    struct arg_lit *all_moves = arg_lit0("A", "all_moves", "All moves");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --move)");
    struct arg_lit *keylist = arg_lit0(NULL, "keylist", "Prints a list of valid fields for --key.");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 2, "Select key");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .AF file");
    struct arg_file *palette = arg_file0("p", "palette", "<file>", "BK file for palette");
    struct arg_lit *play = arg_lit0(NULL, "play", "Play animation or sprite (requires --anim and --palette)");
    struct arg_int *scale = arg_int0(NULL, "scale", "<factor>", "Scales sprites (requires --play)");
    struct arg_lit *parse = arg_lit0(NULL, "parse", "Parse value (requires --key)");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,new,move,all_moves,sprite,keylist,key,value,output,palette,play,scale,parse,end};
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
    if(output->count == 0) {
        if(value->count > 0) {
            printf("--value requires --output\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
    }
    if(play->count == 0) {
        if(scale->count > 0) {
            printf("--scale requires --play\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
    }
    if(palette->count == 0) {
        if(play->count > 0) {
            printf("--play requires --palette\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
    }
    if(file->count == 0 && new->count == 0) {
        printf("Either --file or --new argument required!");
        goto exit_0;
    }
    if(file->count == 1 && new->count == 1) {
        printf("Define at most one of (--file, --new).");
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
    sd_af_file af;
    sd_af_create(&af);
    if(file->count > 0) {
        int ret = sd_af_load(&af, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load AF file! [%d] %s.\n", ret, sd_get_error(ret));
            goto exit_1;
        }
    }
    
    // Palette
    sd_bk_file bk;
    sd_bk_create(&bk);
    if(palette->count > 0) {
        int ret = sd_bk_load(&bk, palette->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load Palette BK file! [%d] %s\n", ret, sd_get_error(ret));
            goto exit_2;
        }
    }

    // Scaling variable
    int _sc = 1;
    if(scale->count > 0) {
        _sc = scale->ival[0];
        if(_sc > 4) _sc = 4;
        if(_sc < 1) _sc = 1;
    }

    // Check args
    if(sprite->count > 0) {
        // Make sure sprite exists.
        if(!check_move_sprite(&af, move->ival[0], sprite->ival[0])) {
            goto exit_2;
        }
        sd_sprite *sp = af.moves[move->ival[0]]->animation->sprites[sprite->ival[0]];
        
        // Handle arguments
        if(key->count > 0) {
            if(value->count > 0) {
                sprite_set_key(sp, key->sval, key->count, value->sval[0]);
            } else {
                sprite_get_key(sp, key->sval, key->count);
            }
        } else if(keylist->count > 0) {
            sprite_keylist();
        } else if(play->count > 0) {
            sprite_play(&af, &bk, _sc, move->ival[0], sprite->ival[0]);
        } else {
            sprite_info(sp, move->ival[0], sprite->ival[0]);
        }
    } else if(move->count > 0) {
        // Make sure the Move exists
        if(!check_move(&af, move->ival[0])) {
            goto exit_2;
        }
        sd_move *mv = af.moves[move->ival[0]];
        sd_animation *ani = mv->animation;
    
        // Handle arguments
        if(key->count > 0) {
            if(value->count > 0) {
                move_set_key(mv, ani, key->sval, key->count, value->sval[0]);
            } else {
                move_get_key(mv, ani, key->sval, key->count, parse->count);
            }
        } else if(keylist->count > 0) {
            move_keylist();
        } else if(play->count > 0) {
            move_play(&af, &bk, _sc, move->ival[0]);
        } else {
            move_info(mv, ani, move->ival[0]);
        }
    } else if(all_moves->count > 0) {
        sd_move *mv;
        sd_animation *ani;
        for(int i = 0; i < 70; i++) {
            if(af.moves[i]) {
                mv = af.moves[i];
                ani = mv->animation;
                if(key->count > 0) {
                    if(value->count > 0) {
                        move_set_key(mv, ani, key->sval, key->count, value->sval[0]);
                    } else {
                        printf("move %2u: ", i);
                        move_get_key(mv, ani, key->sval, key->count, parse->count);
                    }
                } else {
                    printf("\n");
                    move_info(mv, ani, i);
                }
            }
        }
    } else {
        if(key->count > 0) {
            if(value->count > 0) {
                af_set_key(&af, key->sval, key->count, value->sval[0]);
            } else {
                af_get_key(&af, key->sval, key->count);
            }
        } else if(keylist->count > 0) {
            af_keylist();
        } else {
            af_info(&af);
        }
    }

    // Write output file
    if(output->count > 0) {
        sd_af_save(&af, output->filename[0]);
    }
    
    // Quit
exit_2:
    sd_bk_free(&bk);
exit_1:
    sd_af_free(&af);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
