#include <bk.h>
#include <stdio.h>

int main(void) {
    bk_file *file;

    printf("Loading file ...\n");
    file = bk_load("MAIN.BK");

    printf("Destroying resources ...\n");
    bk_destroy(file);

    printf("Exiting.\n");
    return 0;
}
