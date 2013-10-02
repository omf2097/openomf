#include "game/protos/intersect.h"

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
    //vec2i pos_a = object_get_pos(obj);
    //vec2i pos_b = object_get_pos(target);
    //vec2i size_a = object_get_size(obj);
    //vec2i size_b = object_get_size(target);
    int frame_id = obj->cur_animation->id;

    // Iterate through hitpoints
    int hitpixel = 0;
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    while((cc = iter_next(&it)) != NULL) {
        // Skip coords that don't belong to the frame we are checking
        if(cc->frame_index != frame_id) continue;

        // ...

        if(hitpixel >= color_range_start && hitpixel <= color_range_end) {
            return 1;
        }
    }
    return 0;
}
