#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

void roundtrip_sprites(int i, sd_animation *anim, sd_palette *palette) {
    sd_rgba_image *img;
    /*sd_sprite_image *spr;*/
    unsigned int len;
    printf("animation %d with %d frames\n", i, anim->frame_count);
    for(int j = 0; j < anim->frame_count; j++) {
        if (anim->sprites[j]->img->len == 0) {
            printf("warning, 0 length sprite: %d-%d\n", i, j);
            continue;
        }
        if (anim->sprites[j]->missing != 0) {
            continue;
        }
        len = anim->sprites[j]->img->len;
        /*spr = anim->sprites[j];*/
        img = sd_sprite_image_decode(anim->sprites[j]->img, palette, -1);
        anim->sprites[j]->img = sd_sprite_image_encode(img, palette, -1);
        if (len != anim->sprites[j]->img->len) {
            printf("\n");
            printf("XXX sprite length %d changed from %u to %u\n", j, len, anim->sprites[j]->img->len);
            img = sd_sprite_image_decode(anim->sprites[j]->img, palette, -1);
            /*exit(1);*/
        }
        // TODO a setter for the sprite image would be nice, so we don't leak RAM
    }
}


int main(int argc, char **argv) {
    char buf[256];
    char *ext;

    if (argc < 3) {
        printf("Usage %s <input> <output>\n", argv[0]);
        return 1;
    }

    strncpy(buf, argv[1], 255);
    buf[255] = '0';
    basename(buf);

    ext = strrchr(buf, '.');
    if (ext == NULL) {
        printf("cannot determine file extension for %s\n", buf);
        return 1;
    }

    if (strncmp(ext, ".AF", 3) == 0) {
        sd_af_file *file = sd_af_create();
        printf("Loading AF file: %s\n", argv[1]);
        sd_bk_file *bk = sd_bk_create();
        sd_bk_load(bk, "/home/andrew/DOS/OMF/ARENA0.BK");
        if (sd_af_load(file, argv[1]) == SD_SUCCESS) {
            printf("File loaded.\n");
            printf("Writing AF file to %s.\n", argv[2]);
            for(int i = 0; i < 70; i++) {
                if(file->moves[i]) {
                    roundtrip_sprites(i, file->moves[i]->animation, bk->palettes[0]);
                }
            }
            sd_af_save(file, argv[2]);
            sd_af_delete(file);
        } else {
            printf("Unable to load file!\n");
            return 1;
        }
    } else if (strncmp(ext, ".BK", 3) == 0) {
        sd_bk_file *file = sd_bk_create();
        printf("Loading BK file: %s\n", argv[1]);
        if(sd_bk_load(file, argv[1]) == SD_SUCCESS) {
            printf("File loaded.\n");
            printf("Writing BK file to %s.\n", argv[2]);
            for(int i = 0; i < 50; i++) {
                if(file->anims[i]) {
                    roundtrip_sprites(i, file->anims[i]->animation, file->palettes[0]);
                }
            }
            sd_bk_save(file, argv[2]);
            sd_bk_delete(file);
        } else {
            printf("Unable to load file!\n");
        }
    } else {
        printf("Unrecognized file extenion %s\n", ext);
        return 1;
    }
    return 0;
}


