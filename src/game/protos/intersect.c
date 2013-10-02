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

int intersect_sprite_point(object *obj, vec2i point, int range_color_start, int range_color_end) {
    return 0;
}