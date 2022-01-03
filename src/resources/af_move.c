#include <string.h>
#include "formats/move.h"
#include "resources/af_move.h"

void af_move_create(af_move *move, void *src, int id) {
    sd_move *sdmv = (sd_move*)src;
    str_from_c(&move->move_string, sdmv->move_string);
    str_from_c(&move->footer_string, sdmv->footer_string);
    move->id = id;
    move->next_move = sdmv->next_anim_id;
    move->successor_id = sdmv->successor_id;
    move->category = sdmv->category;
    move->damage = sdmv->damage_amount / 2.0f;
    move->points = sdmv->points * 400;
    move->scrap_amount = sdmv->scrap_amount;
    move->pos_constraints = sdmv->unknown_2;
    move->collision_opts = sdmv->unknown_18;
    animation_create(&move->ani, sdmv->animation, id);
}

void af_move_free(af_move *move) {
    animation_free(&move->ani);
    str_free(&move->move_string);
    str_free(&move->footer_string);
}
