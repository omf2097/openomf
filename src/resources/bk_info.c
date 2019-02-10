#include "resources/bk_info.h"
#include "formats/bkanim.h"

void bk_info_create(bk_info *info, void *src, int id) {
    sd_bk_anim *sdinfo = (sd_bk_anim*)src;
    animation_create(&info->ani, sdinfo->animation, id);
    info->chain_hit = sdinfo->chain_hit;
    info->chain_no_hit = sdinfo->chain_no_hit;
    info->load_on_start = sdinfo->load_on_start;
    info->probability = sdinfo->probability;
    info->hazard_damage = sdinfo->hazard_damage / 2;
    str_create_from_cstr(&info->footer_string, sdinfo->footer_string);
}

void bk_info_free(bk_info *info) {
    animation_free(&info->ani);
    str_free(&info->footer_string);
}
