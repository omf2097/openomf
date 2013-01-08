/** @file main.c
  * @brief .AF file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include "../shared/animation_misc.h"
#include "../shared/conversions.h"

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
    
    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", s->pos_x, s->pos_y, s->img->w, s->img->h, s->img->len);
    
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

    sd_rgba_image *img = sd_sprite_image_decode(s->img, bk->palettes[0], -1);

    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img->data, img->w, img->h, 32, img->w*4,
            rmask, gmask, bmask, amask))) {
        printf("Could not create surface: %s\n", SDL_GetError());
        return;
    }

    if ((texture = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    SDL_FreeSurface(surface);
    sd_rgba_image_delete(img);

    rect.x = s->pos_x + 160;
    rect.y = s->pos_y + 100;
    rect.w = s->img->w;
    rect.h = s->img->h;
    
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
                        sprite = (sprite+1) % af->moves[anim]->animation->frame_count;
                        printf("sprite is now %u\n", sprite);
                        changed = 1;
                        break;
                    case SDLK_LEFT:
                        sprite--;
                        if (sprite < 0) {
                            sprite = af->moves[anim]->animation->frame_count - 1;
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
                            printf("UP: animation is now %u\n", anim);
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
                            printf("DOWN: animation is now %u\n", anim);
                            sprite = 0;
                        }
                        changed = 1;
                        break;
                    default:
                        changed = 0;
                }
                if (changed) {
                    s = af->moves[anim]->animation->sprites[sprite];
                    img = sd_sprite_image_decode(s->img, bk->palettes[0], -1);
                    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", s->pos_x, s->pos_y, s->img->w, s->img->h, s->img->len);

                    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img->data, img->w, img->h, 32, img->w*4,
                                    rmask, gmask, bmask, amask))) {
                        printf("Could not create surface: %s\n", SDL_GetError());
                        return;
                    }

                    if ((texture = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
                        printf("Could not create texture: %s\n", SDL_GetError());
                        return;
                    }

                    SDL_FreeSurface(surface);

                    rect.x = s->pos_x + 160;
                    rect.y = s->pos_y + 100;
                    rect.w = s->img->w;
                    rect.h = s->img->h;
                }
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, rendertarget);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderFillRect(renderer, &dstrect);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
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
    if(strcmp(key, "move_footer") == 0) return 14;
    if(strcmp(key, "move_string") == 0) return 15;
    if(strcmp(key, "footer_string") == 0) return 16;
    return anim_key_get_id(key);
}

void move_set_key(sd_move *move, sd_animation *ani, const char **key, int kcount, const char *value) {
    int tmp = 0;
    int kn = move_key_get_id(key[0]);
    switch(kn) {
        case 14: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 21) {
                    move->unknown[tmp] = conv_ubyte(value);
                } else {
                    printf("Move footer index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                printf("Key move_footer requires 1 parameter!\n");
                return;
            }
            break;
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

void move_get_key(sd_move *mv, sd_animation *ani, const char **key, int kcount, int pcount) {
    int tmp = 0;
    int kn = move_key_get_id(key[0]);
    switch(kn) {
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
        case 15:
            printf("%s\n", mv->move_string); break;
        case 16:
            if (pcount > 0 && mv->footer_string) {
                sd_stringparser *parser = sd_stringparser_create();
                int err = sd_stringparser_set_string(parser, ani->anim_string);
                if(err) {
                    char err_msg[255];
                    sd_get_error(err_msg, err);
                    printf("Animation string parser error: %s (%s)\n", err_msg, ani->anim_string);
                } else {
                    sd_stringparser_prettyprint(parser);
                }
                sd_stringparser_delete(parser);
            } else {
                printf("%s\n", mv->footer_string ? mv->footer_string : "(null)");
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
    printf("* move_footer <byte #>\n");
    printf("* move_string <21 chars max>\n");
    printf("* footer_string\n");
}

void move_info(sd_move *mv, sd_animation *ani, int move) {
    printf("Move #%d information:\n\n", move);
    
    anim_common_info(ani);

    printf("\nAF specific footer:\n");
    printf(" * Move footer:     ");
    for(int i = 0; i < 21; i++) {
        printf("%d ", (uint8_t)mv->unknown[i]);
    }
    printf("\n");

    printf(" * Move string:     %s\n", mv->move_string);
    printf(" * Footer string:   %s\n", mv->footer_string);
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
                    af->footer[tmp] = conv_ubyte(value);
                } else {
                    printf("Footer index %d does not exist!\n", tmp);
                    return;
                }
            } else {
                printf("Footer value requires index parameter (eg. --key footer --key 3).\n");
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
    struct arg_lit *play = arg_lit0(NULL, "play", "Play animation or sprite (requires --anim and --palette)");
    struct arg_int *scale = arg_int0(NULL, "scale", "<factor>", "Scales sprites (requires --play)");
    struct arg_lit *parse = arg_lit0(NULL, "parse", "Parse value (requires --key)");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,move,all_moves,sprite,keylist,key,value,output,palette,play,scale,parse,end};
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
    
    // Palette
    sd_bk_file *bk = sd_bk_create();
    if(palette->count > 0) {
        int ret = sd_bk_load(bk, palette->filename[0]);
        if(ret) {
            printf("Unable to load Palette BK file! Make sure the file exists and is a valid BK file.\n");
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
        if(!check_move_sprite(af, move->ival[0], sprite->ival[0])) {
            goto exit_2;
        }
        sd_sprite *sp = af->moves[move->ival[0]]->animation->sprites[sprite->ival[0]];
        
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
            sprite_play(af, bk, _sc, move->ival[0], sprite->ival[0]);
        } else {
            sprite_info(sp, move->ival[0], sprite->ival[0]);
        }
    } else if(move->count > 0) {
        // Make sure the Move exists
        if(!check_move(af, move->ival[0])) {
            goto exit_2;
        }
        sd_move *mv = af->moves[move->ival[0]];
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
            move_play(af, bk, _sc, move->ival[0]);
        } else {
            move_info(mv, ani, move->ival[0]);
        }
    } else if(all_moves->count > 0) {
        sd_move *mv;
        sd_animation *ani;
        for(int i = 0; i < 70; i++) {
            if (af->moves[i]) {
                mv = af->moves[i];
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
                af_set_key(af, key->sval, key->count, value->sval[0]);
            } else {
                af_get_key(af, key->sval, key->count);
            }
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
exit_2:
    sd_bk_delete(bk);
exit_1:
    sd_af_delete(af);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
