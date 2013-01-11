#ifndef _SD_BK_ANIMS
#define _SD_BK_ANIMS

#include <stdint.h>

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;
typedef struct sd_animation_t sd_animation;

typedef struct sd_bk_anim_t {
    uint8_t null;
    uint8_t chain_hit;
    uint8_t chain_no_hit;
    uint8_t load_on_start;
    uint16_t probability;
    uint8_t hazard_damage;
    char* unknown_data;

    sd_animation *animation;
} sd_bk_anim;

sd_bk_anim* sd_bk_anim_create();
void sd_bk_anim_delete(sd_bk_anim *bka);
int sd_bk_anim_load(sd_reader *reader, sd_bk_anim *bka);
void sd_bk_anim_save(sd_writer *writer, sd_bk_anim *bka);

void set_bk_anim_string(sd_bk_anim *bka, const char *data);

#endif // _SD_BK_ANIMS
