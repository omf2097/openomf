#ifndef _PIC_H
#define _PIC_H

#ifdef __cplusplus 
extern "C" {
#endif

#include "shadowdive.h"

#define MAX_PIC_PHOTOS 256

typedef struct sd_pic_photo_t {
    int is_player;
    int sex;
    sd_palette pal;
    sd_sprite *sprite;
} sd_pic_photo;

typedef struct sd_pic_file_t {
    int photo_count;
    sd_pic_photo *photos[MAX_PIC_PHOTOS];
} sd_pic_file;

sd_pic_file* sd_pic_create();
int sd_pic_load(sd_pic_file *pic, const char *filename);
int sd_pic_save(sd_pic_file *pic, const char *filename);
void sd_pic_delete(sd_pic_file *pic);

#ifdef __cplusplus
}
#endif

#endif // _PIC_H
