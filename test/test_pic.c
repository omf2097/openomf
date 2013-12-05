#include <stdlib.h>
#include <stdio.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("test_pic <picfile>\n");
        return 0;
    }

    // Open pic file
    sd_pic_file *pic = sd_pic_create();
    int ret = sd_pic_load(pic, argv[1]);
    if(ret == SD_FILE_OPEN_ERROR) {
        printf("PIC file could not be loaded!\n");
        return 1;
    } else if(ret == SD_FILE_PARSE_ERROR) {
        printf("Invalid file format!\n");
        return 1;
    }

    printf("Loaded file %s\n", argv[1]);
    printf("Photo count: %d\n", pic->photo_count);
    // TODO


    sd_pic_delete(pic);
    return 0;
}