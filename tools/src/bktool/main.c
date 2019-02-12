/** @file main.c
  * @brief .BK file editor tool
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

void bkanim_info(sd_bk_anim *bka, sd_animation *ani, int anim);

int check_anim_sprite(sd_bk_file *bk, int anim, int sprite) {
    if(anim > 50 || anim < 0 || bk->anims[anim] == 0) {
        printf("Animation #%d does not exist.\n", anim);
        return 0;
    }
    if(sprite < 0 || bk->anims[anim]->animation->sprites[sprite] == 0 || sprite >= bk->anims[anim]->animation->sprite_count) {
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

    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", s->pos_x, s->pos_y, s->width, s->height, s->len);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    uint32_t rmask, gmask, bmask, amask;

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    sd_rgba_image img;
    sd_vga_image_decode(&img, bk->background, bk->palettes[0], -1);

    if(!(surface = SDL_CreateRGBSurfaceFrom((void*)img.data, img.w, img.h, 32, img.w*4,
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
    sd_rgba_image_free(&img);

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

    rect.x = s->pos_x;
    rect.y = s->pos_y;
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
                        sprite = (sprite+1) % bk->anims[anim]->animation->sprite_count;
                        printf("sprite is now %u\n", sprite);
                        changed = 1;
                        break;
                    case SDLK_LEFT:
                        sprite--;
                        if (sprite < 0) {
                            sprite = bk->anims[anim]->animation->sprite_count - 1;
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
                            printf("UP: animation is now %d\n", anim);
                            sd_bk_anim *bka = bk->anims[anim];
                            sd_animation *ani = bka->animation;
                            bkanim_info(bka, ani, anim);
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
                            printf("DOWN: animation is now %d\n", anim);
                            sd_bk_anim *bka = bk->anims[anim];
                            sd_animation *ani = bka->animation;
                            bkanim_info(bka, ani, anim);
                            sprite = 0;
                        }
                        changed = 1;
                        break;
                    default:
                        changed = 0;
                }
                if (changed) {
                    s = bk->anims[anim]->animation->sprites[sprite];
                    sd_sprite_rgba_decode(&img, s, bk->palettes[0], -1);
                    int x = s->pos_x + bk->anims[anim]->animation->start_x;
                    int y = s->pos_y + bk->anims[anim]->animation->start_y;
                    printf("Sprite Info: pos=(%d,%d) size=(%d,%d) len=%d\n", x, y, s->width, s->height, s->len);

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

                    rect.x = x;
                    rect.y = y;
                    rect.w = s->width;
                    rect.h = s->height;
                }
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, rendertarget);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        SDL_RenderCopy(renderer, texture, NULL, &rect);

        // render the collision data
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for(int i = 0; i < bk->anims[anim]->animation->coord_count; i++) {
            int x = bk->anims[anim]->animation->coord_table[i].x;
            int y = bk->anims[anim]->animation->coord_table[i].y;
            int frame_id = bk->anims[anim]->animation->coord_table[i].frame_id;
            if(frame_id == sprite) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, rendertarget, NULL, &dstrect);
        SDL_RenderPresent(renderer);
        SDL_Delay(10); // don't chew too much CPU
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
        case 6:  sd_bk_set_anim_string(bka, value); break;
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
        case 6: printf("%s\n", bka->footer_string); break;
        default:
            anim_get_key(ani, kn, key, kcount, pcount);
    }
}

void anim_play(sd_bk_file *bk, int scale, int anim) {
    if(!check_anim(bk, anim)) return;
    sprite_play(bk, scale, anim, 0);
}

void bkanim_push(sd_bk_file *bk, int key) {
    sd_animation ani;
    sd_bk_anim bka;
    sd_animation_create(&ani);
    sd_bk_anim_create(&bka);
    sd_bk_anim_set_animation(&bka, &ani);
    int ret;
    if((ret = sd_bk_set_anim(bk, key, &bka)) != SD_SUCCESS) {
        printf("Could not push new animation: %s.\n", sd_get_error(ret));
        return;
    }
    printf("Pushed empty animation to index %d\n", key);
}

void bkanim_pop(sd_bk_file *bk, int key) {
    sd_bk_set_anim(bk, key, NULL);
    printf("Popped animation from index %d\n", key);
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
    printf(" * String:          %s\n", bka->footer_string);
    printf("\n");

    anim_common_info(ani);
}

// BK Root  --------------------------------------------------------------

int bk_key_get_id(const char* key) {
    if(strcmp(key, "fileid") == 0) return 0;
    if(strcmp(key, "palette") == 0) return 1;
    if(strcmp(key, "unknown") == 0) return 2;
    if(strcmp(key, "footer") == 0) return 3;
    if(strcmp(key, "background") == 0) return 4;
    return -1;
}

void bk_set_key(sd_bk_file *bk, const char **key, int kcount, const char *value) {
    int tmp = 0;
    switch(bk_key_get_id(key[0])) {
        case 0: bk->file_id = conv_udword(value); break;
        case 1: printf("Setting palette not supported."); break;
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
            printf("Value setting not supported for this key!\n");
            return;
    }
    printf("Value set!\n");
}

void bk_get_key(sd_bk_file *bk, const char **key, int kcount) {
    int tmp = 0;
    unsigned char r,g,b;
    switch(bk_key_get_id(key[0])) {
        case 0: printf("%d\n", bk->file_id); break;
        case 1: {
                if(kcount <= 1) {
                    printf("Palette index required for palette fetching.\n");
                    return;
                }
                int index = conv_ubyte(key[1]);
                sd_palette *pal = sd_bk_get_palette(bk, index);
                if(pal == NULL) {
                    printf("No palette found at index %d.\n", index);
                    return;
                }
                for(int tmp = 0; tmp < 256; tmp++) {
                    r = pal->data[tmp][0];
                    g = pal->data[tmp][1];
                    b = pal->data[tmp][2];
                    printf("%d = %3u %3u %3u\n", tmp, r, g, b);
                }
            }
            break;
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
            printf("Value retrieving not supported for this key!\n");
    }
}

void bk_push_key(sd_bk_file *bk, const char **key) {
    switch(bk_key_get_id(key[0])) {
        case 1: {
            sd_palette pal;
            sd_palette_create(&pal);
            sd_bk_push_palette(bk, &pal);
            sd_palette_free(&pal);
            printf("Element pushed; new size is %d.\n", bk->palette_count);
            }
            break;
        default:
            printf("Pushing not supported for this key.\n");
    }
}

void bk_pop_key(sd_bk_file *bk, const char **key) {
    switch(bk_key_get_id(key[0])) {
        case 1:
            sd_bk_pop_palette(bk);
            printf("Element popped; new size is %d.\n", bk->palette_count);
            break;
        default:
            printf("Popping not supported for this key.\n");
    }
}

void bk_export_key(sd_bk_file *bk, const char **key, int kcount, const char *filename) {
    switch(bk_key_get_id(key[0])) {
        case 1: {
            if(kcount <= 1) {
                printf("Palette index required for palette exporting.\n");
                return;
            }
            int index = atoi(key[1]);
            sd_palette *pal = sd_bk_get_palette(bk, index);
            if(pal == NULL) {
                printf("No palette found at index %d.\n", index);
                return;
            }
            int ret = sd_palette_to_gimp_palette(pal, filename);
            if(ret != SD_SUCCESS) {
                printf("Error while exporting palette: %s.", sd_get_error(ret));
                return;
            }
            }
            break;
        case 4: {
            sd_palette *pal = sd_bk_get_palette(bk, 0);
            if(pal == NULL) {
                printf("Palette required for exporting to PNG.\n");
                return;
            }
            sd_vga_image *img = sd_bk_get_background(bk);
            if(img == NULL) {
                printf("Background is not set; cannot export.\n");
                return;
            }
            int ret = sd_vga_image_to_png(img, pal, filename);
            if(ret != SD_SUCCESS) {
                printf("Error while exporting background to %s: %s\n",
                    filename,
                    sd_get_error(ret));
                return;
            }
            }
            break;
        default:
            printf("Exporting not supported for this key.\n");
    }
}

void bk_import_key(sd_bk_file *bk, const char **key, int kcount, const char *filename) {
    switch(bk_key_get_id(key[0])) {
        case 1: {
            if(kcount <= 1) {
                printf("Palette index required for palette importing.\n");
                return;
            }
            int index = atoi(key[1]);
            sd_palette *pal = sd_bk_get_palette(bk, index);
            if(pal == NULL) {
                printf("No palette found at index %d.\n", index);
                return;
            }
            int ret = sd_palette_from_gimp_palette(pal, filename);
            if(ret != SD_SUCCESS) {
                printf("Error while importing palette: %s.", sd_get_error(ret));
                return;
            }
            }
            break;
        case 4: {
            sd_vga_image img;
            int ret = sd_vga_image_from_png(&img, filename);
            if(ret != SD_SUCCESS) {
                printf("Error while attempting to import %s: %s\n",
                    filename,
                    sd_get_error(ret));
                return;
            }
            sd_bk_set_background(bk, &img);
            sd_vga_image_free(&img);
            }
            break;
        default:
            printf("Importing not supported for this key.\n");
    }
}

void bk_keylist() {
    printf("Valid field keys for BK file root:\n");
    printf("* fileid\n");
    printf("* palette [<palette_index>]\n");
    printf("* unknown\n");
    printf("* soundtable <byte #>\n");
    printf("* animation\n");
    printf("* background\n");
}

void bk_info(sd_bk_file *bk) {
    printf("BK File information:\n");
    printf(" * File ID:     %d\n", bk->file_id);
    printf(" * Palettes:    %d\n", bk->palette_count);
    printf(" * Unknown A:   %d\n", bk->unknown_a);

    printf(" * Animations:  ");
    int start = -1, last = -1;
    unsigned int m;
    for(m = 0; m < 50; m++) {
        if(bk->anims[m]) {
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
            if(start == -1) {
                printf("0\n");
            } else {
                printf("%d\n", last);
            }
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
        printf("%2d ", bk->soundtable[k]);
    }
    printf("|\n");
}

// Main --------------------------------------------------------------

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "Input .BK file");
    struct arg_lit *new = arg_lit0("n", "new", "Creates a new structure for editing.");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .BK file");
    struct arg_int *anim = arg_int0("a", "anim", "<animation_id>", "Select animation");
    struct arg_lit *all_anims = arg_lit0("A", "all_anims", "All animations");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --anim)");
    struct arg_lit *keylist = arg_lit0(NULL, "keylist", "Prints a list of valid fields for --key.");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 2, "Select key");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_lit *play = arg_lit0(NULL, "play", "Play animation or sprite (requires --anim)");
    struct arg_lit *push = arg_lit0(NULL, "push", "Push a new element (requires --key)");
    struct arg_lit *pop = arg_lit0(NULL, "pop", "Pop the last element (requires --key)");
    struct arg_file *export = arg_file0(NULL, "export", "<file>", "Exports data to a file (requires --key)");
    struct arg_file *import = arg_file0(NULL, "import", "<file>", "Imports data from a file (requires --key)");
    struct arg_int *stencil = arg_int0(NULL, "stencil", "<int>", "Stencil index for image (requires --import)");
    struct arg_int *scale = arg_int0(NULL, "scale", "<factor>", "Scales sprites (requires --play)");
    struct arg_lit *parse = arg_lit0(NULL, "parse", "Parse value (requires --key)");
    struct arg_end *end = arg_end(30);
    void* argtable[] = {help,vers,file,new,output,anim,all_anims,sprite,keylist,key,value,push,pop,export,import,stencil,play,scale,parse,end};
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
    if(file->count == 0 && new->count == 0) {
        printf("Either --file or --new argument required!");
        goto exit_0;
    }
    if(file->count == 1 && new->count == 1) {
        printf("Define at most one of (--file, --new).");
        goto exit_0;
    }
    if(push->count == 1 && pop->count == 1) {
        printf("Define at most one of (--push, --pop).");
        goto exit_0;
    }
    if(export->count == 1 && import->count == 1) {
        printf("Define at most one of (--export, --import).");
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
    sd_bk_file bk;
    sd_bk_create(&bk);
    if(file->count > 0) {
        int ret = sd_bk_load(&bk, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load BK file! [%d] %s.\n", ret, sd_get_error(ret));
            goto exit_1;
        }
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
        sd_animation *ani;
        sd_sprite *sp;

        // Make sure the bkanim exists
        if(!check_anim(&bk, anim->ival[0])) {
            goto exit_1;
        }

        // This doesn't need a sprite check
        ani = bk.anims[anim->ival[0]]->animation;
        if(push->count > 0) {
            anim_push(ani);
            goto done;
        }

        // Make sure sprite exists.
        if(!check_anim_sprite(&bk, anim->ival[0], sprite->ival[0])) {
            goto exit_1;
        }
        sp = ani->sprites[sprite->ival[0]];

        // Handle arguments
        if(key->count > 0) {
            if(value->count > 0) {
                sprite_set_key(sp, key->sval, key->count, value->sval[0]);
            } else if(export->count > 0) {
                sprite_export_key(sp, key->sval, key->count, export->filename[0], &bk);
            } else if(import->count > 0) {
                int st = -1;
                if(stencil->count > 0) {
                    st = stencil->ival[0];
                }
                sprite_import_key(sp, key->sval, key->count, import->filename[0], st);
            } else {
                sprite_get_key(sp, key->sval, key->count);
            }
        } else if(keylist->count > 0) {
            sprite_keylist();
        } else if(play->count > 0) {
            sprite_play(&bk, _sc, anim->ival[0], sprite->ival[0]);
        } else if(pop->count > 0) {
            anim_pop(ani);
        } else {
            sprite_info(sp, anim->ival[0], sprite->ival[0]);
        }
    } else if(anim->count > 0) {
        // This doesn't need a check
        if(push->count > 0) {
            bkanim_push(&bk, anim->ival[0]);
            goto done;
        }

        // Make sure the bkanim exists
        if(!check_anim(&bk, anim->ival[0])) {
            goto exit_1;
        }
        sd_bk_anim *bka = bk.anims[anim->ival[0]];
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
            anim_play(&bk, _sc, anim->ival[0]);
        } else if(pop->count > 0) {
            bkanim_pop(&bk, anim->ival[0]);
        } else {
            bkanim_info(bka, ani, anim->ival[0]);
        }
    } else if(all_anims->count > 0) {
        sd_bk_anim *bka;
        sd_animation *ani;
        for(int i = 0; i < 50; i++) {
            if (bk.anims[i]) {
                bka = bk.anims[i];
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
    } else if(new->count > 0) {
        printf("Creating a new BK structure.\n");
    } else {
        if(key->count > 0) {
            if(value->count > 0) {
                bk_set_key(&bk, key->sval, key->count, value->sval[0]);
            } else if(push->count > 0) {
                bk_push_key(&bk, key->sval);
            } else if(pop->count > 0) {
                bk_pop_key(&bk, key->sval);
            } else if(export->count > 0) {
                bk_export_key(&bk, key->sval, key->count, export->filename[0]);
            } else if(import->count > 0) {
                bk_import_key(&bk, key->sval, key->count, import->filename[0]);
            } else {
                bk_get_key(&bk, key->sval, key->count);
            }
        } else if(keylist->count > 0) {
            bk_keylist();
        } else {
            bk_info(&bk);
        }
    }

done:
    // Write output file
    if(output->count > 0) {
        int ret = sd_bk_save(&bk, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Error attempting to save to %s: %s\n",
                output->filename[0],
                sd_get_error(ret));
        }
    }

    // Quit
exit_1:
    sd_bk_free(&bk);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
