#include "utils/vec.h"
#include <math.h>

vec2i vec2i_add(vec2i a, vec2i b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

vec2i vec2i_sub(vec2i a, vec2i b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

vec2i vec2i_mult(vec2i a, vec2i b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

vec2f vec2f_add(vec2f a, vec2f b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

vec2f vec2f_sub(vec2f a, vec2f b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

vec2f vec2f_mult(vec2f a, vec2f b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

vec2i vec2f_to_i(vec2f f) {
    vec2i i;
    i.x = f.x;
    i.y = f.y;
    return i;
}

vec2f vec2i_to_f(vec2i i) {
    vec2f f;
    f.x = i.x;
    f.y = i.y;
    return f;
}

vec2f vec2f_norm(vec2f a) {
    float mag = vec2f_mag(a);
    a.x /= mag;
    a.y /= mag;
    return a;
}

float vec2f_mag(vec2f a) {
    return sqrtf(a.x * a.x + a.y * a.y);
}

float vec2f_dist(vec2f a, vec2f b) {
    return vec2f_mag(vec2f_sub(a, b));
}

vec2i vec2i_create(int x, int y) {
    vec2i v;
    v.x = x;
    v.y = y;
    return v;
}

vec2f vec2f_create(float x, float y) {
    vec2f v;
    v.x = x;
    v.y = y;
    return v;
}
