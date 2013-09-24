#include "resources/sprite.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void sprite_create_custom(sprite *sp, vec2i pos, void *raw_sprite) {
	sp->pos = pos;
	sp->raw_sprite = raw_sprite;
	texture_create(&sp->tex, NULL, 0, 0);
}

void sprite_create(sprite *sp, void *src) {
	sd_sprite *sdsprite = (sd_sprite*)src;
	sp->raw_sprite = (void*)sd_sprite_vga_decode(sdsprite->img);
	texture_create(&sp->tex, NULL, 0, 0);
    sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
}

void sprite_init(sprite *sp, palette *pal, int remap_id) {
	sd_rgba_image *img = sd_vga_image_decode(
							(sd_vga_image*)sp->raw_sprite, 
							(sd_palette*)pal, 
							remap_id);
	texture_free(&sp->tex);
	texture_create(&sp->tex, img->data, img->w, img->h);
	sd_rgba_image_delete(img);
}

void sprite_free(sprite *sp) {
	texture_free(&sp->tex);
	sd_vga_image_delete((sd_vga_image*)sp->raw_sprite);
}