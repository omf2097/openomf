#include "game/physics/collisions.h"
#include "game/physics/shape_rect.h"
#include "game/physics/shape_invrect.h"

// Right now we only use invrect in limiting the arena
// And only type of invrects we have are static
// TODO: Implement other cases as required..
void collision_rect_invrect(object *a, object *b) {
	shape *as = a->col_shape_hard;
	shape *bs = b->col_shape_hard;

	if(b->is_static) {
		if((a->pos.x + shape_rect_get_size(as).x) >= (b->pos.x + shape_invrect_get_size(bs).x)) {
			object_set_px(a, (b->pos.x + shape_invrect_get_size(bs).x) - shape_rect_get_size(as).x);
			object_set_vx(a, 0.0f);
			object_reset_vstate(a);
		} else if(a->pos.x < b->pos.x) {
			object_set_px(a, b->pos.x);
			object_set_vx(a, 0.0f);
			object_reset_vstate(a);
		}

		if((a->pos.y + shape_rect_get_size(as).y) >= (b->pos.y + shape_invrect_get_size(bs).y)) {
			object_set_py(a, (b->pos.y + shape_invrect_get_size(bs).y - shape_rect_get_size(as).y));
			object_set_vy(a, 0.0f);
			object_reset_hstate(a);
		} else if(a->pos.y <= b->pos.y) {
			object_set_py(a, b->pos.y);
			object_set_vy(a, 0.1f);
			object_reset_hstate(a);
		}
	}
}

void collision_rect_rect(object *a, object *b) {

}

void collision_point_rect(object *a, object *b) {

}

void collision_point_invrect(object *a, object *b) {

}

void collision_point_point(object *a, object *b) {

}

void collision_handle(object *ao, object *bo) {
	// Handle generic stuff

	// Nothing right now

	// Handle shape specific stuff
	shape *a = ao->col_shape_hard;
	shape *b = bo->col_shape_hard;
    switch(a->type) {
        case SHAPE_TYPE_RECT:
            switch(b->type) {
            	case SHAPE_TYPE_RECT: collision_rect_rect(ao, bo);
                case SHAPE_TYPE_POINT: collision_point_rect(bo, ao);
                case SHAPE_TYPE_INVRECT: collision_rect_invrect(ao, bo);
            };
            break;
        case SHAPE_TYPE_POINT:
            switch(b->type) {
            	case SHAPE_TYPE_POINT: collision_point_point(ao, bo);
                case SHAPE_TYPE_RECT: collision_point_rect(ao, bo);
                case SHAPE_TYPE_INVRECT: collision_point_invrect(ao, bo);
            };
            break;
        case SHAPE_TYPE_INVRECT:
            switch(b->type) {
                case SHAPE_TYPE_POINT: collision_point_invrect(bo, ao);
                case SHAPE_TYPE_RECT: collision_rect_invrect(bo, ao);
            };
            break;
    };
}