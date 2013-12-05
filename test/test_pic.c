#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

#define UNUSED(x) (void)(x)

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("test_pic <picfile> [-d basename]\n");
        return 0;
    }
    int dump = 0;
    char *basename = 0;
    if(argc == 4) {
        if(strcmp(argv[2], "-d") == 0) {
            dump = 1;
            basename = (char*)argv[3];
        }
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

    if(dump) {
        printf("Dump flag used! Dumping sprites ...\n");
    }

    for(int i = 0; i < pic->photo_count; i++) {
        if(!dump) {
            printf("Photo %d:\n", i);
            printf("  * Length = %d\n", 
                pic->photos[i]->sprite->img->len);
            printf("  * Size = (%d,%d)\n", 
                pic->photos[i]->sprite->img->w,
                pic->photos[i]->sprite->img->h);
            printf("  * Position = (%d,%d)\n", 
                pic->photos[i]->sprite->pos_x,
                pic->photos[i]->sprite->pos_y);
            printf("  * Sex = %d\n", 
                pic->photos[i]->sex);
            printf("  * Is Player = %d\n", 
                pic->photos[i]->is_player);
        } else {
            char filename[128];
            sprintf(filename, "%s_%d.ppm", basename, i);
            printf(" * Dumping to '%s' ... ", filename);
            sd_rgba_image *img = sd_sprite_image_decode(
                pic->photos[i]->sprite->img,
                &pic->photos[i]->pal,
                -1);
            sd_rgba_image_to_ppm(img, filename);
            sd_rgba_image_delete(img);
            printf("OK.\n");
        }
    }


    sd_pic_delete(pic);
    return 0;
}