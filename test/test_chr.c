#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("test_chr <chrfile>\n");
        return 0;
    }

    // Open pic file
    sd_chr_file *chr = sd_chr_create();
    int ret = sd_chr_load(chr, argv[1]);
    if(ret != SD_SUCCESS) {
        printf("Shadowdive error %d: %s\n", ret, sd_get_error(ret));
        return 1;
    } 

    // Dump file header data
    printf("Loaded file '%s'\n", argv[1]);
    
    // TODO

    sd_chr_delete(chr);
    return 0;
}
