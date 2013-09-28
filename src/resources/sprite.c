#include "resources/sprite.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void sprite_create_custom(sprite *sp, vec2i pos, void *raw_sprite) {
	sp->pos = pos;
	sp->raw_sprite = raw_sprite;
	texture_create(&sp->tex);
}

void sprite_create(sprite *sp, void *src) {
	sd_sprite *sdsprite = (sd_sprite*)src;
	sp->raw_sprite = (void*)sd_sprite_vga_decode(sdsprite->img);
	sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
	texture_create(&sp->tex);
}

void sprite_init(sprite *sp, palette *pal, int remap_id) {
	sd_rgba_image *img = sd_vga_image_decode(
							(sd_vga_image*)sp->raw_sprite, 
							(sd_palette*)pal, 
							remap_id);
	if(texture_init(&sp->tex, img->data, img->w, img->h)) {
		PERROR("sprite_init: Error while creating texture!");
	}
	sd_rgba_image_delete(img);
}

void sprite_reinit(sprite *sp, palette *pal, int remap_id) {
	sd_rgba_image *img = sd_vga_image_decode(
							(sd_vga_image*)sp->raw_sprite, 
							(sd_palette*)pal, 
							remap_id);
	if(texture_upload(&sp->tex, img->data)) {
		PERROR("sprite_reinit: Error while reinitializing sprite texture!");
	}
	sd_rgba_image_delete(img);
}

void sprite_free(sprite *sp) {
	texture_free(&sp->tex);
	sd_vga_image_delete((sd_vga_image*)sp->raw_sprite);
}
