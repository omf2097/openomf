#include <bk.h>
#include <af.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

void print_sprites(int i, sd_animation *anim, sd_palette *palette) {
    char buf[256];
    printf("animation %d with %d frames\n", i, anim->frame_count);
    for(int j = 0; j < anim->frame_count; j++) {
        if (anim->sprites[j]->missing > 0) {
            printf("missing sprite: %d-%d\n", i, j);
            continue;
        }
        if (anim->sprites[j]->img->len == 0) {
            printf("warning, 0 length sprite: %d-%d\n", i, j);
            continue;
        }
        sprintf(buf, "sprite-%d-%d.ppm", i, j);
        /*printf("decoding sprite to %s\n", buf);*/
        sd_rgba_image_to_ppm(sd_sprite_image_decode(anim->sprites[j]->img, palette, -1), buf);
    }
}

int main(int argc, char **argv) {
    char buf[256];
    char *ext;

    if (argc <= 1) {
        printf("Usage %s [-p palette]  <filename>\n", argv[0]);
        return 1;
    }

    strncpy(buf, argv[argc-1], 255);
    buf[255] = '0';
    basename(buf);

    ext = strrchr(buf, '.');
    if (ext == NULL) {
        printf("cannot determine file extension for %s\n", buf);
        return 1;
    }

    if (strncmp(ext, ".AF", 3) == 0) {
        if (argc < 3 || strncmp(argv[1], "-p", 2)) {
            printf("AF files need a corresponding palette, please supply one with -p (eg ARENA0.BK)\n");
            return 1;
        }

        sd_bk_file *bk_file;
        sd_af_file *file;
        printf("Loading AF file: %s\n", argv[argc-1]);
        file = sd_af_load(argv[argc-1]);
        if (file) {
            printf("File loaded.\n");
            printf("Reading palette from %s\n", argv[2]);
            bk_file = sd_bk_load(argv[2]);
            if (bk_file) {
                printf("Palette loaded.\n");
                for(int i = 0; i < 50; i++) {
                    if (file->moves[i]) {
                        print_sprites(i, &file->moves[i]->animation, bk_file->palettes[0]);
                    }
                }
                printf("Destroying resources ...\n");
                sd_bk_delete(bk_file);
                sd_af_delete(file);
            } else {
                printf("Unable to load palette from file.\n");
                return 1;
            }
        } else {
            printf("Unable to load file!\n");
            return 1;
        }
        return 0;
    } else if (strncmp(ext, ".BK", 3) == 0) {
        sd_bk_file *file;
        printf("Loading BK file: %s\n", argv[1]);
        file = sd_bk_load(argv[1]);
        if(file) {
            printf("File loaded.\n");
            printf("ID: %d\n", file->file_id);
            for (int i = 0; i < file->num_palettes; i++) {
                printf("drawing background with pallete %d to background-%d.ppm\n", i, i);
                sprintf(buf, "background-%d.ppm", i);
                sd_rgba_image_to_ppm(sd_vga_image_decode(file->background, file->palettes[i], -1), buf);
            }

            for(int i = 0; i < 50; i++) {
                if (file->anims[i]) {
                    print_sprites(i, file->anims[i]->animation, file->palettes[0]);
                } else {
                    /*printf("skipping blank animation %d\n", i);*/
                }
            }

            printf("Destroying resources ...\n");
            sd_bk_delete(file);
        } else {
            printf("Unable to load file!\n");
        }
    } else {
        printf("Unrecognized file extenion %s\n", ext);
        return 1;
    }

    printf("Exiting.\n");
    return 0;
}
