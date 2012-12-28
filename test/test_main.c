#include <bk.h>
#include <stdio.h>

int main(void) {
    sd_bk_file *file;
    char buf[255];

    printf("Loading file ...\n");
    file = sd_bk_load("resources/MAIN.BK");
    if(file) {
        printf("File loaded.\n");
        printf("ID: %d\n", file->file_id);
        for (int i = 0; i < file->num_palettes; i++) {
            printf("drawing background with pallete %d to background-%d.ppm\n", i, i);
            sprintf(buf, "background-%d.ppm", i);
            sd_rgba_image_to_ppm(sd_vga_image_decode(file->background, file->palettes[i], -1), buf);
        }

        for(int i = 0; i < 50; i++) {
            if (file->animations[i]) {
                printf("animation %d with %d frames\n", i, file->animations[i]->frame_count);
                for(int j = 0; j < file->animations[i]->frame_count; j++) {
                    if (file->animations[i]->sprites[j]->missing > 0) {
                        printf("missing sprite: %d-%d\n", i, j);
                        continue;
                    }
                    if (file->animations[i]->sprites[j]->img->len == 0) {
                        printf("warning, 0 length sprite: %d-%d\n", i, j);
                        continue;
                    }
                    sprintf(buf, "sprite-%d-%d.ppm", i, j);
                    /*printf("decoding sprite to %s\n", buf);*/
                    sd_rgba_image_to_ppm(sd_sprite_image_decode(file->animations[i]->sprites[j]->img, file->palettes[0], -1), buf);
                }
            } else {
                /*printf("skipping blank animation %d\n", i);*/
            }
        }

        printf("Destroying resources ...\n");
        sd_bk_delete(file);
    } else {
        printf("Unable to load file!\n");
    }

    printf("Exiting.\n");
    return 0;
}
