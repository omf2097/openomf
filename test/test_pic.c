#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

#define UNUSED(x) (void)(x)

int main(int argc, char *argv[]) {
    if(argc < 2) {
        printf("test_pic <picfile> [-d basename bkfile]\n");
        return 0;
    }
    int dump = 0;
    char *basename = 0;
    char *bkfile = 0;
    if(argc == 5) {
        if(strcmp(argv[2], "-d") == 0) {
            dump = 1;
            basename = (char*)argv[3];
            bkfile = (char*)argv[4];
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

    printf("Loaded file '%s'\n", argv[1]);
    
    sd_bk_file *bk = NULL;
    if(dump) {
        bk = sd_bk_create();
        if(sd_bk_load(bk, bkfile) != SD_SUCCESS) {
            printf("Could not load BK file!\n");
            goto error_0;
        } else {
            printf("Loaded file '%s'.\n", bkfile);
        }
        printf("Dump flag used! Dumping sprites ...\n");
    }

    printf("Photo count: %d\n", pic->photo_count);

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
                bk->palettes[0],
                -1);
            sd_rgba_image_to_ppm(img, filename);
            sd_rgba_image_delete(img);
            printf("OK.\n");
        }
    }

    if(dump) {
        sd_bk_delete(bk);
    }
    sd_pic_delete(pic);
    return 0;

error_0:
    sd_pic_delete(pic);
    return 1;
}
