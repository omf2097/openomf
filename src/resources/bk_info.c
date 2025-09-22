#include "resources/bk_info.h"
#include "formats/bkanim.h"
#include "resources/modmanager.h"

void bk_info_create(int file_id, bk_info *info, array *sprites, void *src, int id) {
    sd_bk_anim *sdinfo = (sd_bk_anim *)src;
    animation_create(BK_ANIMATION, file_id, &info->ani, sprites, sdinfo->animation, id);
    // TODO check for mod data here
    info->chain_hit = sdinfo->chain_hit;
    info->chain_no_hit = sdinfo->chain_no_hit;
    info->load_on_start = sdinfo->load_on_start;
    info->probability = sdinfo->probability;
    info->hazard_damage = sdinfo->hazard_damage;
    str_from_c(&info->footer_string, sdinfo->footer_string);
    modmanager_get_bk_animation(file_id, id, info);
}

void bk_info_free(bk_info *info) {
    animation_free(&info->ani);
    str_free(&info->footer_string);
}
