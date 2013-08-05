#include "game/physics/intersect.h"
#include "game/physics/shape_rect.h"
#include "game/physics/shape_invrect.h"

int intersect_point_rect(vec2i pc, vec2i rc, vec2i size) {
    return (
        pc.x > rc.x && 
        pc.x < (rc.x + size.x) &&
        pc.y > rc.y && 
        pc.y < (rc.y + size.y)
    );
}

int intersect_point_invrect(vec2i pc, vec2i rc, vec2i size) {
    return (
        pc.x > (rc.x + size.x) || 
        pc.x < rc.x ||
        pc.y > (rc.y + size.y) ||
        pc.y < rc.y
    );
}

int intersect_rect_rect(vec2i rac, vec2i ras, vec2i rbc, vec2i rbs) {
    return !(
        rac.x > (rbc.x + rbs.x) ||
        rac.y > (rbc.y + rbs.y) ||
        (rac.x + ras.x) < rbc.x ||
        (rac.y + ras.y) < rbc.y
    );
}

int intersect_rect_invrect(vec2i rac, vec2i ras, vec2i rbc, vec2i rbs) {
    return (
        (rac.x + ras.x) > (rbc.x + rbs.x) ||
        (rac.y + ras.y) > (rbc.y + rbs.y) ||
        rac.x < rbc.x ||
        rac.y < rbc.y
    );
}

int intersect_point_point(vec2i a, vec2i b) {
    return (
        a.x == b.x && 
        a.y == b.y
    );
}

int shape_intersect(shape *a, vec2i ca, shape *b, vec2i cb) {
    if(a->type == b->type) {
        switch(a->type) {
            case SHAPE_TYPE_RECT: return intersect_rect_rect(ca, shape_rect_get_size(a), cb, shape_rect_get_size(b));
            case SHAPE_TYPE_POINT: return intersect_point_point(ca, cb);
        };
    }

    switch(a->type) {
        case SHAPE_TYPE_RECT:
            switch(b->type) {
                case SHAPE_TYPE_POINT: return intersect_point_rect(cb, ca, shape_rect_get_size(a));
                case SHAPE_TYPE_INVRECT: return intersect_rect_invrect(ca, shape_rect_get_size(a), cb, shape_invrect_get_size(b));
            };
            break;
        case SHAPE_TYPE_POINT:
            switch(b->type) {
                case SHAPE_TYPE_RECT: return intersect_point_rect(ca, cb, shape_rect_get_size(b));
                case SHAPE_TYPE_INVRECT: return intersect_point_invrect(ca, cb, shape_invrect_get_size(b));
            };
            break;
    };
    return 0;
}
