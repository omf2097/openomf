#include "resources/af_move.h"
#include "formats/move.h"

void af_move_create(af_move *move, array *sprites, void *src, int id) {
    sd_move *sdmv = (sd_move *)src;
    str_from_c(&move->move_string, sdmv->move_string);
    str_from_c(&move->footer_string, sdmv->footer_string);
    move->id = id;
    move->next_move = sdmv->next_anim_id;
    move->successor_id = sdmv->successor_id;
    move->category = sdmv->category;
    move->damage = sdmv->damage_amount;
    move->raw_damage = sdmv->damage_amount;
    move->stun = 0.0f;
    move->points = sdmv->points * 400;
    move->block_damage = sdmv->block_damage;
    move->block_stun = sdmv->block_stun;
    move->pos_constraints = sdmv->pos_constraint;
    move->collision_opts = sdmv->collision_opts;
    move->extra_string_selector = sdmv->extra_string_selector;
    animation_create(&move->ani, sprites, sdmv->animation, id);
    if(id == ANIM_JUMPING) {
        // fixup the jump coordinates
        animation_fixup_coordinates(&move->ani, 0, -50);
    }
}

void af_move_free(af_move *move) {
    animation_free(&move->ani);
    str_free(&move->move_string);
    str_free(&move->footer_string);
}
