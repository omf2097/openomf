#ifndef _SHAPE_H
#define _SHAPE_H

typedef struct shape_t {
    void *data;
    int (*check_collision)(void *data, vec2f check_pos, vec2f self_pos);
    int (*free)(void *data);
} shape;

int shape_check_collision(void *shape, vec2f check_pos, vec2f self_pos);
void shape_free(void *shape);

#endif // _SHAPE_H