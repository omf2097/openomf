#include "resources/sprite.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void sprite_create_custom(sprite *sp, vec2i pos, void *raw_sprite) {
	sp->initialized = 0;
	sp->pos = pos;
	sp->raw_sprite = raw_sprite;
}

void sprite_create(sprite *sp, void *src) {
	sp->initialized = 0;
	sd_sprite *sdsprite = (sd_sprite*)src;
	sp->raw_sprite = (void*)sd_sprite_vga_decode(sdsprite->img);
	sp->pos = vec2i_create(sdsprite->pos_x, sdsprite->pos_y);
}

void sprite_init(sprite *sp, palette *pal, int remap_id) {
	sd_rgba_image *img = sd_vga_image_decode(
							(sd_vga_image*)sp->raw_sprite, 
							(sd_palette*)pal, 
							remap_id);
	if(texture_create(&sp->tex, img->data, img->w, img->h)) {
		PERROR("There was an error while creating texture in sprite_init!");
	}
	sd_rgba_image_delete(img);
	sp->initialized = 1;
}

void sprite_free(sprite *sp) {
	if(sp->initialized) {
		sp->initialized = 0;
		texture_free(&sp->tex);
	}
	sd_vga_image_delete((sd_vga_image*)sp->raw_sprite);
}
