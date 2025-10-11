#include "resources/animation.h"
#include "formats/animation.h"
#include "resources/modmanager.h"
#include "utils/allocator.h"
#include <stdlib.h>

typedef struct sprite_reference_t {
    sprite *sprite;
} sprite_reference;

void animation_create(animation_source type, int file_id, animation *ani, array *sprites, void *src, int id) {
    sd_animation *sdani = (sd_animation *)src;

    // Copy simple stuff
    ani->id = id;
    ani->start_pos = vec2i_create(sdani->start_x, sdani->start_y);
    str_from_c(&ani->animation_string, sdani->anim_string);

    // Copy collision coordinates
    vector_create_with_size(&ani->collision_coords, sizeof(collision_coord), sdani->coord_count);
    collision_coord tmp_coord;
    for(int i = 0; i < sdani->coord_count; i++) {
        tmp_coord.pos = vec2i_create(sdani->coord_table[i].x, sdani->coord_table[i].y);
        tmp_coord.frame_index = sdani->coord_table[i].frame_id;
        vector_append(&ani->collision_coords, &tmp_coord);
    }

    ani->extra_string_count = sdani->extra_string_count;
    // Copy extra strings
    vector_create_with_size(&ani->extra_strings, sizeof(str), sdani->extra_string_count);
    str tmp_string;
    for(int i = 0; i < sdani->extra_string_count; i++) {
        str_from_c(&tmp_string, sdani->extra_strings[i]);
        vector_append(&ani->extra_strings, &tmp_string);
        // don't str_free tmp_str here because it will free the pointers
        // inside it, which vector_append does not copy
    }

    // Handle sprites
    vector_create_with_size(&ani->sprites, sizeof(sprite_reference), sdani->sprite_count);
    sprite *tmp_sprite;
    for(int i = 0; i < sdani->sprite_count; i++) {
        if(sdani->sprites[i]->missing) {
            // read the right index from the sprite table
            tmp_sprite = omf_calloc(1, sizeof(sprite));
            sprite_create_reference(tmp_sprite, (void *)sdani->sprites[i], i,
                                    ((sprite *)array_get(sprites, sdani->sprites[i]->index))->data);
            sprite_reference spr;
            spr.sprite = tmp_sprite;
            vector_append(&ani->sprites, &spr);
        } else {
            tmp_sprite = omf_calloc(1, sizeof(sprite));
            sd_sprite *sp;
            // TODO check the mod overrides for a replacement sprite
            if(modmanager_get_sprite(type, file_id, ani->id, i, &sp)) {
                sprite_create(tmp_sprite, (void *)sp, i);
                tmp_sprite->data->render_w = sdani->sprites[i]->width;
                tmp_sprite->data->render_h = sdani->sprites[i]->height;
                tmp_sprite->pos.x = sdani->sprites[i]->pos_x;
                tmp_sprite->pos.y = sdani->sprites[i]->pos_y;
            } else {
                sprite_create(tmp_sprite, (void *)sdani->sprites[i], i);
            }
            sprite_reference spr;
            spr.sprite = tmp_sprite;
            if(sdani->sprites[i]->index) {
                // insert into the global sprite table
                array_set(sprites, sdani->sprites[i]->index, tmp_sprite);
            }
            vector_append(&ani->sprites, &spr);
        }
    }
}

animation *create_animation_from_single(sprite *sp, vec2i pos) {
    animation *a = omf_calloc(1, sizeof(animation));
    a->start_pos = pos;
    a->id = -1;
    str_from_c(&a->animation_string, "A9999999999");
    vector_create_with_size(&a->collision_coords, sizeof(collision_coord), 0);
    vector_create_with_size(&a->extra_strings, sizeof(str), 0);
    vector_create_with_size(&a->sprites, sizeof(sprite_reference), 1);
    sprite_reference spr;
    spr.sprite = sp;
    vector_append(&a->sprites, &spr);
    return a;
}

int animation_clone(animation *src, animation *dst) {
    iterator it;
    memcpy(dst, src, sizeof(animation));
    str_from(&dst->animation_string, &src->animation_string);
    vector_create_with_size(&dst->collision_coords, sizeof(collision_coord), vector_size(&src->collision_coords));
    vector_iter_begin(&src->collision_coords, &it);
    collision_coord *tmp_coord = NULL;
    foreach(it, tmp_coord) {
        vector_append(&dst->collision_coords, tmp_coord);
    }
    str *tmp_str = NULL;
    vector_create_with_size(&dst->extra_strings, sizeof(str), vector_size(&src->extra_strings));
    vector_iter_begin(&src->extra_strings, &it);
    foreach(it, tmp_str) {
        str new_str;
        str_create(&new_str);
        str_from(&new_str, tmp_str);
        vector_append(&dst->extra_strings, &new_str);
    }
    vector_create_with_size(&dst->sprites, sizeof(sprite_reference), vector_size(&src->sprites));
    vector_iter_begin(&src->sprites, &it);
    sprite_reference *spr = NULL;
    foreach(it, spr) {
        sprite_reference spr_clone;
        spr_clone.sprite = sprite_copy(spr->sprite);
        vector_append(&dst->sprites, &spr_clone);
    }

    return 0;
}

void animation_fixup_coordinates(animation *ani, int fix_x, int fix_y) {
    iterator it;
    sprite_reference *spr;
    // Fix sprite positions
    vector_iter_begin(&ani->sprites, &it);
    foreach(it, spr) {
        spr->sprite->pos.x += fix_x;
        spr->sprite->pos.y += fix_y;
    }
    // Fix collisions coordinates
    collision_coord *c;
    vector_iter_begin(&ani->collision_coords, &it);
    foreach(it, c) {
        c->pos.x += fix_x;
        c->pos.y += fix_y;
    }
}

sprite *animation_get_sprite(animation *ani, int sprite_id) {
    sprite_reference *s = (sprite_reference *)vector_get(&ani->sprites, sprite_id);
    if(s == NULL) {
        return NULL;
    }
    return s->sprite;
}

int animation_get_sprite_count(animation *ani) {
    return vector_size(&ani->sprites);
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
    foreach(it, tmp_str) {
        str_free(tmp_str);
    }
    vector_free(&ani->extra_strings);

    vector_iter_begin(&ani->sprites, &it);
    sprite_reference *spr;
    foreach(it, spr) {
        sprite_free(spr->sprite);
        omf_free(spr->sprite);
    }

    vector_free(&ani->sprites);
}
