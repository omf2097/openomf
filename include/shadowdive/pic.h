#ifndef _SD_PIC_H
#define _SD_PIC_H

#include "shadowdive/sprite.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define MAX_PIC_PHOTOS 256

typedef struct {
    int is_player;
    int sex;
    sd_palette pal;
    sd_sprite *sprite;
} sd_pic_photo;

typedef struct {
    int photo_count;
    sd_pic_photo *photos[MAX_PIC_PHOTOS];
} sd_pic_file;

int sd_pic_create(sd_pic_file *pic);
int sd_pic_load(sd_pic_file *pic, const char *filename);
int sd_pic_save(sd_pic_file *pic, const char *filename);
void sd_pic_free(sd_pic_file *pic);

#ifdef __cplusplus
}
#endif

#endif // _SD_PIC_H
