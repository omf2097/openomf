#ifndef _SD_BK_ANIMS
#define _SD_BK_ANIMS

#include <stdint.h>
#include "shadowdive/animation.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    uint8_t null;
    uint8_t chain_hit;
    uint8_t chain_no_hit;
    uint8_t load_on_start;
    uint16_t probability;
    uint8_t hazard_damage;
    char* unknown_data;

    sd_animation *animation;
} sd_bk_anim;

int sd_bk_anim_create(sd_bk_anim *bka);
int sd_bk_anim_copy(sd_bk_anim *dst, const sd_bk_anim *src);
void sd_bk_anim_free(sd_bk_anim *bka);

void set_bk_anim_string(sd_bk_anim *bka, const char *data);

#ifdef SD_USE_INTERNAL
int sd_bk_anim_load(sd_reader *reader, sd_bk_anim *bka);
void sd_bk_anim_save(sd_writer *writer, const sd_bk_anim *bka);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_BK_ANIMS
