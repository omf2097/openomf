#include <bk.h>
#include <stdio.h>

int main(void) {
    sd_bk_file *file;

    printf("Loading file ...\n");
    file = sd_bk_load("resources/MAIN.BK");
    if(file) {
        printf("File loaded.\n");
        printf("ID: %d\n", file->file_id);

        printf("Destroying resources ...\n");
        sd_bk_delete(file);
    } else {
        printf("Unable to load file!\n");
    }

    printf("Exiting.\n");
    return 0;
}
