#include "resources/af_move.h"
#include <shadowdive/shadowdive.h>

void af_move_create(af_move *move, void *src) {
	sd_move *sdmv = (sd_move*)src;
	str_create_from_data(&move->move_string, sdmv->move_string, 21);
	str_create_from_cstr(&move->footer_string, sdmv->footer_string);
	animation_create(&move->ani, sdmv->animation);
}

void af_move_free(af_move *move) {
	animation_free(&move->ani);
	str_free(&move->move_string);
	str_free(&move->footer_string);
}