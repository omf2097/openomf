#include "utils/vec.h"

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