#include "resources/bk_info.h"
#include "formats/bkanim.h"
#include "resources/modmanager.h"

void bk_info_create(str *name, bk_info *info, array *sprites, void *src, int id) {
    sd_bk_anim *sdinfo = (sd_bk_anim *)src;
    animation_create(BK_ANIMATION, name, &info->ani, sprites, sdinfo->animation, id);
    info->chain_hit = sdinfo->chain_hit;
    info->chain_no_hit = sdinfo->chain_no_hit;
    info->load_on_start = sdinfo->load_on_start;
    info->probability = sdinfo->probability;
    info->hazard_damage = sdinfo->hazard_damage;
    str_from_c(&info->footer_string, sdinfo->footer_string);
    modmanager_get_bk_animation(name, id, info);
}

void bk_info_free(bk_info *info) {
    animation_free(&info->ani);
    str_free(&info->footer_string);
}
