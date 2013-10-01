#include "game/physics/intersect.h"
#include "game/physics/shape_rect.h"
#include "game/physics/shape_invrect.h"
#include "utils/log.h"

int intersect_object_object(object *a, object *b) {
    vec2i pos_a = object_get_pos(a);
    vec2i pos_b = object_get_pos(b);
    return !(
        rac.x > (rbc.x + rbs.x) ||
        rac.y > (rbc.y + rbs.y) ||
        (rac.x + ras.x) < rbc.x ||
        (rac.y + ras.y) < rbc.y
    );
}

int intersect_object_point(object *obj, object *b)