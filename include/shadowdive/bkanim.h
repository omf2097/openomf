#ifndef _BK_ANIMS
#define _BK_ANIMS

#include "shadowdive/animation.h"

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;

typedef struct sd_bk_anim_t {
    uint8_t null;
    uint8_t unknown_a;
    uint8_t unknown_b;
    uint8_t unknown_c;
    uint16_t unknown_d;
    uint8_t unknown_e;
    uint16_t unknown_size;
    char* unknown_data;

    sd_animation *animation;
} sd_bk_anim;

sd_bk_anim* sd_bk_anim_create();
void sd_bk_anim_delete(sd_bk_anim *bka);
int sd_bk_anim_load(sd_reader *reader, sd_bk_anim *bka);
void sd_bk_anim_save(sd_writer *writer, sd_bk_anim *bka);

#endif // _BK_ANIMS
