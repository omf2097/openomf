#include "resources/sprite.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void sprite_create_custom(sprite *sp, vec2i pos, void *raw_sprite) {
	sp->pos = pos;
	sp->raw_sprite = raw_sprite;
}

void sprite_create(sprite *sp, void *src) {
	sd_sprite *sdsprite = (sd_sprite*)src;
	sp->raw_sprite = (void*)sd_sprite_vga_decode(sdsprite->img);
	sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
}

void sprite_free(sprite *sp) {
	sd_vga_image_delete((sd_vga_image*)sp->raw_sprite);
	sp->raw_sprite = NULL;
}

sprite* sprite_copy(sprite *src) {
	sprite *new = malloc(sizeof(sprite));
	new->pos = src->pos;
	new->raw_sprite = sd_vga_image_clone((sd_vga_image*)src->raw_sprite);
	return new;
}