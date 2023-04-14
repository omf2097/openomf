#ifndef VEC_H
#define VEC_H

typedef struct vec2f_t {
    float x;
    float y;
} vec2f;

typedef struct vec2i_t {
    int x;
    int y;
} vec2i;

vec2i vec2i_add(vec2i a, vec2i b);
vec2i vec2i_sub(vec2i a, vec2i b);
vec2i vec2i_mult(vec2i a, vec2i b);

vec2f vec2f_add(vec2f a, vec2f b);
vec2f vec2f_sub(vec2f a, vec2f b);
vec2f vec2f_mult(vec2f a, vec2f b);

vec2f vec2f_norm(vec2f a);
float vec2f_mag(vec2f a);
float vec2f_dist(vec2f a, vec2f b);

vec2i vec2f_to_i(vec2f f);
vec2f vec2i_to_f(vec2i i);

vec2i vec2i_create(int x, int y);
vec2f vec2f_create(float x, float y);

#endif // VEC_H
