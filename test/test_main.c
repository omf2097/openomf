#include <bk.h>
#include <stdio.h>

int main(void) {
    sd_bk_file *file;

    printf("Loading file ...\n");
    file = sd_bk_load("resources/MAIN.BK");
    if(file) {
        printf("File loaded.\n");
        printf("ID: %d\n", file->file_id);

        printf("drawing background to out.ppm\n");
        sd_rgba_image_to_ppm(sd_vga_image_decode(file->background, file->palettes[0], -1), "out.ppm");

        printf("Destroying resources ...\n");
        sd_bk_delete(file);
    } else {
        printf("Unable to load file!\n");
    }

    printf("Exiting.\n");
    return 0;
}
