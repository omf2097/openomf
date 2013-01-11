/** @file main.c
  * @brief .BK file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include "../shared/animation_misc.h"
#include "../shared/conversions.h"

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

// Sprites -------------------------------------------------------

void sprite_play(sd_bk_file *bk, int scale, int anim, int sprite) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Texture *background;
    SDL_Texture *rendertarget;
    SDL_Rect rect;
    SDL_Rect dstrect;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
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

    sd_rgba_image *img = sd_vga_image_decode(bk->background, bk->palettes[0], -1);

    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img->data, img->w, img->h, 32, img->w*4,
            rmask, gmask, bmask, amask))) {
        printf("Could not create surface: %s\n", SDL_GetError());
        return;
    }

    if ((background = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }
    
    if((rendertarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 200)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    SDL_FreeSurface(surface);
    sd_rgba_image_delete(img);

    img = sd_sprite_image_decode(s->img, bk->palettes[0], -1);

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

    rect.x = s->pos_x;
    rect.y = s->pos_y;
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
                        sprite = (sprite+1) % bk->anims[anim]->animation->frame_count;
                        printf("sprite is now %u\n", sprite);
                        changed = 1;
                        break;
                    case SDLK_LEFT:
                        sprite--;
                        if (sprite < 0) {
                            sprite = bk->anims[anim]->animation->frame_count - 1;
                        }
                        changed = 1;
                        break;
                    case SDLK_UP:
                        i++;
                        while (!check_anim(bk, i) && i < 50) {
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
                        while (!check_anim(bk, i) && i >= 0) {
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
                    s = bk->anims[anim]->animation->sprites[sprite];
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

                    rect.x = s->pos_x;
                    rect.y = s->pos_y;
                    rect.w = s->img->w;
                    rect.h = s->img->h;
                }
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, rendertarget);
        SDL_RenderCopy(renderer, background, NULL, NULL);
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

// Animations --------------------------------------------------------------

int bkanim_key_get_id(const char* key) {
    if(strcmp(key, "null") == 0) return 0;
    if(strcmp(key, "chain_hit") == 0) return 1;
    if(strcmp(key, "chain_no_hit") == 0) return 2;
    if(strcmp(key, "load_on_start") == 0) return 3;
    if(strcmp(key, "probability") == 0) return 4;
    if(strcmp(key, "hazard_damage") == 0) return 5;
    if(strcmp(key, "bk_str") == 0) return 6;
    return anim_key_get_id(key);
}

void bkanim_set_key(sd_bk_anim *bka, sd_animation *ani, const char **key, int kcount, const char *value) {
    int kn = bkanim_key_get_id(key[0]);
    switch(kn) {
        case 0:  bka->null = conv_ubyte(value); break;
        case 1:  bka->chain_hit = conv_ubyte(value); break;
        case 2:  bka->chain_no_hit = conv_ubyte(value); break;
        case 3:  bka->load_on_start = conv_ubyte(value); break;
        case 4:  bka->probability = conv_uword(value); break;
        case 5:  bka->hazard_damage = conv_ubyte(value); break;
        case 6:  set_bk_anim_string(bka, value); break;
        default:
            anim_set_key(ani, kn, key, kcount, value);
            return;
    }
    printf("Value set!\n");
}

void bkanim_get_key(sd_bk_anim *bka, sd_animation *ani, const char **key, int kcount, int pcount) {
    int kn = bkanim_key_get_id(key[0]);
    
    switch(kn) {
        case 0: printf("%d\n", bka->null); break;
        case 1: printf("%d\n", bka->chain_hit); break;
        case 2: printf("%d\n", bka->chain_no_hit); break;
        case 3: printf("%d\n", bka->load_on_start); break;
        case 4: printf("%d\n", bka->probability); break;
        case 5: printf("%d\n", bka->hazard_damage); break;
        case 6: printf("%s\n", bka->unknown_data ? bka->unknown_data : "(null)"); break;
        default:
            anim_get_key(ani, kn, key, kcount, pcount);
    }
}

void anim_play(sd_bk_file *bk, int scale, int anim) {
    if(!check_anim(bk, anim)) return;
    sprite_play(bk, scale, anim, 0);
}

void bkanim_keylist() {
    printf("Valid field keys for Animation structure:\n");
    printf("* null\n");
    printf("* chain_hit\n");
    printf("* chain_no_hit\n");
    printf("* load_on_start\n");
    printf("* probability\n");
    printf("* hazard_damage\n");
    printf("* bk_str\n");
    anim_keylist();
}

void bkanim_info(sd_bk_anim *bka, sd_animation *ani, int anim) {
    printf("Animation #%d information:\n", anim);
    
    printf("\nBK specific header:\n");
    printf(" * Null:            %d\n", bka->null);
    printf(" * Chain # if hit:  %d\n", bka->chain_hit);
    printf(" * Chain # not hit: %d\n", bka->chain_no_hit);
    printf(" * Load on start:   %d\n", bka->load_on_start);
    printf(" * Probability:     %d\n", bka->probability);
    printf(" * hazard damage:   %d\n", bka->hazard_damage);
    printf(" * String:          %s\n", bka->unknown_data);
    printf("\n");
    
    anim_common_info(ani);
}

// BK Root  --------------------------------------------------------------

int bk_key_get_id(const char* key) {
    if(strcmp(key, "fileid") == 0) return 0;
    //if(strcmp(key, "palette") == 0) return 1;
    if(strcmp(key, "unknown") == 0) return 2;
    if(strcmp(key, "footer") == 0) return 3;
    return -1;
}

void bk_set_key(sd_bk_file *bk, const char **key, int kcount, const char *value) {
    int tmp = 0;
    switch(bk_key_get_id(key[0])) {
        case 0: bk->file_id = conv_udword(value); break;
        case 1: break; // TODO
        case 2: bk->unknown_a = conv_ubyte(value); break;
        case 3: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 30) {
                    bk->soundtable[tmp] = conv_ubyte(value);
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

void bk_get_key(sd_bk_file *bk, const char **key, int kcount) {
    int tmp = 0;
    switch(bk_key_get_id(key[0])) {
        case 0: printf("%d\n", bk->file_id); break;
        case 1: printf("\n"); break; // TODO
        case 2: printf("%d\n", bk->unknown_a); break;
        case 3: 
            if(kcount == 2) {
                tmp = conv_ubyte(key[1]);
                if(tmp < 30) {
                    printf("%d\n", bk->soundtable[tmp]);
                } else {
                    printf("Soundtable index %d does not exist!\n", tmp);
                }
            } else {
                for(int i = 0; i < 30; i++) { printf("%d ", bk->soundtable[i]); } printf("\n"); 
            }
            break;
        default:
            printf("Unknown key!\n");
    }
}

void bk_keylist() {
    printf("Valid field keys for BK file root:\n");
    printf("* fileid\n");
    //printf("* palette:<palette #>\n");
    printf("* unknown\n");
    printf("* soundtable <byte #>\n");
}

void bk_info(sd_bk_file *bk) {
    printf("BK File information:\n");
    printf(" * File ID:     %d\n", bk->file_id);
    printf(" * Palettes:    %d\n", bk->num_palettes);
    printf(" * Unknown A:   %d\n", bk->unknown_a);
    
    printf(" * Animations:  ");
    int start = -1, last = -1;
    int m;
    for(m = 0; m < 50; m++) {
        if(bk->anims[m]) {
            if(start == -1) {
                start = m;
                last = m;
            }
            if(m > last+1) {
                if(start == last) {
                    printf("%u, ", last);
                } else {
                    printf("%u-%u, ", start, last);
                }
                start = m;
            }
            last = m;
        }
    }
    if(m != last) {
        if(start == last) {
            printf("%u\n", last);
        } else {
            printf("%u-%u\n", start, last);
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
        printf("%2d ", bk->soundtable[k]);
    }
    printf("|\n");
}

// Main --------------------------------------------------------------

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .BK file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .BK file");
    struct arg_int *anim = arg_int0("a", "anim", "<animation_id>", "Select animation");
    struct arg_lit *all_anims = arg_lit0("A", "all_anims", "All animations");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --anim)");
    struct arg_lit *keylist = arg_lit0(NULL, "keylist", "Prints a list of valid fields for --key.");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 2, "Select key");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_lit *play = arg_lit0(NULL, "play", "Play animation or sprite (requires --anim)");
    struct arg_int *scale = arg_int0(NULL, "scale", "<factor>", "Scales sprites (requires --play)");
    struct arg_lit *parse = arg_lit0(NULL, "parse", "Parse value (requires --key)");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,anim,all_anims,sprite,keylist,key,value,play,scale,parse,end};
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
    
    // Scaling variable
    int _sc = 1;
    if(scale->count > 0) {
        _sc = scale->ival[0];
        if(_sc > 4) _sc = 4;
        if(_sc < 1) _sc = 1;
    }
    
    // Handle args
    if(sprite->count > 0) {
        // Make sure sprite exists.
        if(!check_anim_sprite(bk, anim->ival[0], sprite->ival[0])) {
            goto exit_1;
        }
        sd_sprite *sp = bk->anims[anim->ival[0]]->animation->sprites[sprite->ival[0]];
    
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
            sprite_play(bk, _sc, anim->ival[0], sprite->ival[0]);
        } else {
            sprite_info(sp, anim->ival[0], sprite->ival[0]);
        }
    } else if(anim->count > 0) {
        // Make sure the bkanim exists
        if(!check_anim(bk, anim->ival[0])) {
            goto exit_1;
        }
        sd_bk_anim *bka = bk->anims[anim->ival[0]];
        sd_animation *ani = bka->animation;
    
        if(key->count > 0) {
            if(value->count > 0) {
                bkanim_set_key(bka, ani, key->sval, key->count, value->sval[0]);
            } else {
                bkanim_get_key(bka, ani, key->sval, key->count, parse->count);
            }
        } else if(keylist->count > 0) {
            bkanim_keylist();
        } else if(play->count > 0) {
            anim_play(bk, _sc, anim->ival[0]);
        } else {
            bkanim_info(bka, ani, anim->ival[0]);
        }
    } else if(all_anims->count > 0) {
        sd_bk_anim *bka;
        sd_animation *ani;
        for(int i = 0; i < 50; i++) {
            if (bk->anims[i]) {
                bka = bk->anims[i];
                ani = bka->animation;
                if(key->count > 0) {
                    if(value->count > 0) {
                        bkanim_set_key(bka, ani, key->sval, key->count, value->sval[0]);
                    } else {
                        printf("Animation %2u: ", i);
                        bkanim_get_key(bka, ani, key->sval, key->count, parse->count);
                    }
                } else {
                    printf("\n");
                    bkanim_info(bka, ani, i);
                }
            }
        }
    } else {
        if(key->count > 0) {
            if(value->count > 0) {
                bk_set_key(bk, key->sval, key->count, value->sval[0]);
            } else {
                bk_get_key(bk, key->sval, key->count);
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
