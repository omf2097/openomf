#include <shadowdive/rgba_image.h>
#include "game/protos/intersect.h"
#include "utils/log.h"

int intersect_object_object(object *a, object *b) {
    vec2i pos_a = object_get_pos(a);
    vec2i pos_b = object_get_pos(b);
    vec2i size_a = object_get_size(a);
    vec2i size_b = object_get_size(b);
    return !(
        pos_a.x > (pos_b.x + size_b.x) ||
        pos_a.y > (pos_b.y + size_b.y) ||
        (pos_a.x + size_a.x) < pos_b.x ||
        (pos_a.y + size_a.y) < pos_b.y);
}

int intersect_object_point(object *obj, vec2i point) {
    vec2i pos = object_get_pos(obj);
    vec2i size = object_get_size(obj);
    return (
        point.x < (pos.x + size.x) &&
        point.y < (pos.y + size.y) &&
        point.x > pos.x &&
        point.y > pos.y);
}

int intersect_sprite_hitpoint(object *obj, object *target, int color_range_start, int color_range_end) {
    // Make sure there are hitpoints to check.
    if(vector_size(&obj->cur_animation->collision_coords) == 0) {
        return 0;
    }

    // Some useful variables
    vec2i pos_a = object_get_pos(obj);
    vec2i pos_b = object_get_pos(target);
    vec2i pos_cb = vec2i_add(pos_b, target->cur_sprite->pos);
    vec2i size_b = object_get_size(target);
    vec2i off = vec2i_create(pos_cb.x - pos_a.x, pos_b.y - pos_a.y);
    int frame_id = obj->cur_animation->id;

    if(object_get_direction(target) == -1) {
        pos_cb.x = pos_b.x + ((target->cur_sprite->pos.x * object_get_direction(target)) - size_b.x);
    }

    // Iterate through hitpoints
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    while((cc = iter_next(&it)) != NULL) {
        // Skip coords that don't belong to the frame we are checking
        if(cc->frame_index != frame_id) continue;

        // Get pixel coordinates
        int xcoord = (cc->pos.x * obj->direction) - off.x;
        int ycoord = (size_b.y + (cc->pos.y - off.y)) - (size_b.y + target->cur_sprite->pos.y);
        if(object_get_direction(target) == -1) {
            xcoord = size_b.x - xcoord;
        }

        unsigned char hitpixel = ((sd_vga_image*)obj->cur_sprite->raw_sprite)->data[ycoord * size_b.x + xcoord];
        DEBUG("-- %d", hitpixel);
        if(hitpixel >= color_range_start && hitpixel <= color_range_end) {
            return 1;
        }
    }
    return 0;
}
