#include "resources/animation.h"
#include <shadowdive/shadowdive.h>
#include <stdlib.h>

void animation_create(animation *ani, void *src) {
	sd_animation *sdani = (sd_animation*)src;

	// Copy simple stuff
	ani->start_pos = vec2i_create(sdani->start_x, sdani->start_y);
	str_create_from_cstr(&ani->animation_string, sdani->anim_string);

	// Copy collision coordinates
	vector_create(&ani->collision_coords, sizeof(collision_coord));
	collision_coord tmp_coord;
	for(int i = 0; i < sdani->col_coord_count; i++) {
		tmp_coord.pos = vec2i_create(sdani->col_coord_table[i].x, sdani->col_coord_table[i].y);
		tmp_coord.frame_index = sdani->col_coord_table[i].y_ext;
		vector_append(&ani->collision_coords, &tmp_coord);
	}

	// Copy extra strings
	vector_create(&ani->extra_strings, sizeof(str));
	str tmp_string;
	for(int i = 0; i < sdani->extra_string_count; i++) {
		str_create_from_cstr(&tmp_string, sdani->extra_strings[i]);
		vector_append(&ani->extra_strings, &tmp_string);
		str_free(&tmp_string);
	}

	// Handle sprites
    vector_create(&ani->sprites, sizeof(sprite));
    sprite tmp_sprite;
    for(int i = 0; i < sdani->frame_count; i++) {
	    sprite_create(&tmp_sprite, (void*)sdani->sprites[i]);
	    vector_append(&ani->sprites, &tmp_sprite);
    }
}

sprite* animation_get_sprite(animation *ani, int sprite_id) {
	return (sprite*)vector_get(&ani->sprites, sprite_id);
}

void animation_init(animation *ani, palette *pal, int remap_id) {
	iterator it;
    vector_iter_begin(&ani->sprites, &it);
    sprite *tmp_sprite = NULL;
    while((tmp_sprite = iter_next(&it)) != NULL) {
        sprite_init(tmp_sprite, pal, remap_id);
    }
}

void animation_free(animation *ani) {
	iterator it;

	// Free animation string
	str_free(&ani->animation_string);

	// Free collision coordinates
	vector_free(&ani->collision_coords);

	// Free extra strings
	vector_iter_begin(&ani->extra_strings, &it);
	str *tmp_str = NULL;
	while((tmp_str = iter_next(&it)) != NULL) {
		str_free(tmp_str);
	}
	vector_free(&ani->extra_strings);

	// Free animations
    vector_iter_begin(&ani->sprites, &it);
    sprite *tmp_sprite = NULL;
    while((tmp_sprite = iter_next(&it)) != NULL) {
        sprite_free(tmp_sprite);
    }
    vector_free(&ani->sprites);
}
