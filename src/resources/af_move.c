#include <string.h>
#include <shadowdive/shadowdive.h>
#include "resources/af_move.h"

void af_move_create(af_move *move, void *src, int id) {
    sd_move *sdmv = (sd_move*)src;
    str_create_from_cstr(&move->move_string, sdmv->move_string);
    str_create_from_cstr(&move->footer_string, sdmv->footer_string);
    move->id = id;
    move->next_move = sdmv->unknown[12];
    move->successor_id = sdmv->unknown[16];
    move->category = sdmv->unknown[13];
    move->damage = sdmv->unknown[17] / 2.0f;
    move->points = sdmv->unknown[20] * 400;
    move->scrap_amount = sdmv->unknown[15];
    animation_create(&move->ani, sdmv->animation, id);
#ifdef DEBUGMODE
    memcpy(move->unknown, sdmv->unknown, sizeof(move->unknown));
#endif
}

void af_move_free(af_move *move) {
    animation_free(&move->ani);
    str_free(&move->move_string);
    str_free(&move->footer_string);
}
