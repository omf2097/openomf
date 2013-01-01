#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

int check_anim_sprite(sd_bk_file *bk, int anim, int sprite) {
    if(bk->anims[anim] == 0) {
        printf("Animation #%d does not exist.\n", anim);
        return 0;
    }
    if(bk->anims[anim]->animation->sprites[sprite]) {
        printf("Sprite #%d does not exist.\n", sprite);
        return 0;
    }
    return 1;
}

int check_anim(sd_bk_file *bk, int anim) {
    if(bk->anims[anim] == 0) {
        printf("Animation #%d does not exist.\n", anim);
        return 0;
    }
    return 1;
}

void sprite_set_key(sd_bk_file *bk, int anim, int sprite, const char *key, const char *value) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
}

void sprite_get_key(sd_bk_file *bk, int anim, int sprite, const char *key) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
}

void sprite_play(sd_bk_file *bk, int anim, int sprite) {
    if(!check_anim_sprite(bk, anim, sprite)) return;
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
}

void sprite_info(sd_bk_file *bk, int anim, int sprite) {
    if(!check_anim_sprite(bk, anim, sprite)) return;

    printf("Animation #%d, Sprite #%d information:\n", anim, sprite);
    sd_sprite *s = bk->anims[anim]->animation->sprites[sprite];
}

void anim_set_key(sd_bk_file *bk, int anim, const char *key, const char *value) {
    if(!check_anim(bk, anim)) return;
    
}

void anim_get_key(sd_bk_file *bk, int anim, const char *key) {
    if(!check_anim(bk, anim)) return;
    
}

void anim_play(sd_bk_file *bk, int anim) {
    if(!check_anim(bk, anim)) return;
    
}

void anim_info(sd_bk_file *bk, int anim) {
    if(!check_anim(bk, anim)) return;
    printf("Animation #%d information:\n", anim);
}

void bk_set_key(sd_bk_file *bk, const char *key, const char *value) {

}

void bk_get_key(sd_bk_file *bk, const char *key) {

}

void bk_info(sd_bk_file *bk) {
    printf("BK File information:\n");
    printf(" * File ID: %d\n", bk->file_id);
    printf(" * Palettes: %d\n", bk->num_palettes);
    printf(" * Unknown A: %d\n", bk->unknown_a);
    
    printf("Valid animations:\n");
    for(int i = 0; i < 50; i++) {
        if(bk->anims[i])
            printf(" * %d\n", i);
    }
    
    printf("Footer (hex):\n");
    for(int k = 0; k < 3; k++) {
        for(int i = 0; i < 10; i++) {
            printf("%x\t", bk->footer[i+k*10]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", ".BK file");
    struct arg_int *anim = arg_int0("a", "anim", "<animation_id>", "Select animation");
    struct arg_int *sprite = arg_int0("s", "sprite", "<sprite_id>", "Select sprite (requires --anim)");
    struct arg_str *key = arg_str0(NULL, "key", "<key>", "Select key (requires --anim)");
    struct arg_str *value = arg_str0(NULL, "value", "<value>", "Set value (requires --key)");
    struct arg_str *play = arg_str0(NULL, "play", "<id>", "Play animation or sprite (requires --anim)");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,anim,sprite,key,value,play,end};
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
        printf("'%s' Command line BK file editor.\n", progname);
        goto exit_0;
    }
    
    // Argument dependencies
    if(anim->count == 0) {
        if(sprite->count > 0) {
            printf("--sprite requires --anim\n");
            printf("Try '%s --help' for more information.\n", progname);
            goto exit_0;
        }
        if(key->count > 0) {
            printf("--key requires --anim\n");
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
        } else {
            bk_info(bk);
        }
    }
    
    // Quit
exit_1:
    sd_bk_delete(bk);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}