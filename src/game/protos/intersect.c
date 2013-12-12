#include <shadowdive/rgba_image.h>
#include "game/protos/intersect.h"
#include "utils/log.h"

int intersect_object_object(object *a, object *b) {
    if(a->cur_sprite == NULL || b->cur_sprite == NULL) return 0;
    vec2i pos_a = vec2i_add(object_get_pos(a), a->cur_sprite->pos);
    vec2i pos_b = vec2i_add(object_get_pos(b), b->cur_sprite->pos);
    vec2i size_a = object_get_size(a);
    vec2i size_b = object_get_size(b);
    return !(
        pos_a.x > (pos_b.x + size_b.x) ||
        pos_a.y > (pos_b.y + size_b.y) ||
        (pos_a.x + size_a.x) < pos_b.x ||
        (pos_a.y + size_a.y) < pos_b.y);
}

int intersect_object_point(object *obj, vec2i point) {
    if(obj->cur_sprite == NULL) return 0;
    vec2i pos = vec2i_add(object_get_pos(obj), obj->cur_sprite->pos);
    vec2i size = object_get_size(obj);
    return (
        point.x < (pos.x + size.x) &&
        point.y < (pos.y + size.y) &&
        point.x > pos.x &&
        point.y > pos.y);
}

#ifdef DEBUGMODE
int intersect_sprite_hitpoint(object *obj, object *target, int level, vec2i *point, image *di) 
#else
int intersect_sprite_hitpoint(object *obj, object *target, int level, vec2i *point) 
#endif
{
    // Make sure both objects have sprites going
    if(obj->cur_sprite == NULL || target->cur_sprite == NULL) {
        return 0;
    }
    // Make sure there are hitpoints to check.
    if(vector_size(&obj->cur_animation->collision_coords) == 0) {
        return 0;
    }

    // Some useful variables
    vec2i pos_a = vec2i_add(object_get_pos(obj), obj->cur_sprite->pos);
    vec2i pos_b = vec2i_add(object_get_pos(target), target->cur_sprite->pos);
    vec2i size_a = object_get_size(obj);
    vec2i size_b = object_get_size(target);

    if (object_get_direction(obj) == OBJECT_FACE_LEFT) {
        pos_a.x = object_get_pos(obj).x + ((obj->cur_sprite->pos.x * -1) - size_a.x);
    }

    if (object_get_direction(target) == OBJECT_FACE_LEFT) {
        pos_b.x = object_get_pos(target).x + ((target->cur_sprite->pos.x * -1) - size_b.x);
    }


    // Iterate through hitpoints
    vec2i hcoords[level];
    int found = 0;
    iterator it;
    collision_coord *cc;
    vector_iter_begin(&obj->cur_animation->collision_coords, &it);
    while((cc = iter_next(&it)) != NULL) {
        // Skip coords that don't belong to the frame we are checking
        if(cc->frame_index != obj->cur_sprite->id) continue;

        // Convert coords to target sprite local space
        int t = (object_get_direction(obj) == OBJECT_FACE_RIGHT)
            ? (pos_a.x + cc->pos.x - obj->cur_sprite->pos.x)
            : (pos_a.x + (size_a.x - cc->pos.x) + obj->cur_sprite->pos.x);

        // convert global coordinates to local coordinates by compensating for the other player's position
        int xcoord = t - pos_b.x;
        // Also note that the hit pixel position during jumps is innacurate because hacks
        int ycoord = (pos_a.y + size_a.y + cc->pos.y) - pos_b.y;

        ycoord -= (obj->cur_sprite->pos.y + size_a.y);

// TODO: Use correct coordinates
#ifdef DEBUGMODE
        image_set_pixel(
            di,
            xcoord + pos_b.x,
            ycoord + pos_b.y,
            color_create(32,255,32,255));
        image_rect(di, pos_b.x, pos_b.y, size_b.x, size_b.y, color_create(255,32,32,255));
        image_rect(di, pos_a.x, pos_a.y, size_a.x, size_a.y, color_create(32,32,255,255));
#endif

        // Make sure that the hitpixel is within the area of the target sprite
        if(xcoord < 0 || xcoord >= size_b.x) continue;
        if(ycoord < 0 || ycoord >= size_b.y) continue;

#ifdef DEBUGMODE
        image_set_pixel(
            di,
            xcoord + pos_b.x,
            ycoord + pos_b.y,
            color_create(255,32,32,255));
#endif

        // Get hitpixel
        sd_vga_image *vga = (sd_vga_image*)target->cur_sprite->raw_sprite;
        int hitpoint = (ycoord * vga->w) + xcoord;
        if (object_get_direction(target) == OBJECT_FACE_LEFT) {
            hitpoint = (ycoord * vga->w) + (vga->w - xcoord);
        }
        if(vga->stencil[hitpoint] == 1) {
            hcoords[found++] = vec2i_create(xcoord, ycoord);
            if(found >= level) {
                vec2f sum = vec2f_create(0,0);
                for(int k = 0; k < level; k++) {
                    sum.x += hcoords[k].x;
                    sum.y += hcoords[k].y;
                }
                point->x = (sum.x / level) + pos_b.x;
                point->y = (sum.y / level) + pos_b.y;
                return 1;
            }
        }
    }

    return 0;
}
