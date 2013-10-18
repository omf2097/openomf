#include "resources/af_move.h"
#include <shadowdive/shadowdive.h>

void af_move_create(af_move *move, void *src, int id) {
	sd_move *sdmv = (sd_move*)src;
	str_create_from_cstr(&move->move_string, sdmv->move_string);
	str_create_from_cstr(&move->footer_string, sdmv->footer_string);
	animation_create(&move->ani, sdmv->animation, id);
}

void af_move_free(af_move *move) {
	animation_free(&move->ani);
	str_free(&move->move_string);
	str_free(&move->footer_string);
}