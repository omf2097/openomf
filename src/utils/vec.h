#ifndef VEC_H
#define VEC_H

#include "fixedptc.h"
#include <math.h>

typedef struct vec2f {
    fixedpt fx;
    fixedpt fy;
} vec2f;

typedef struct vec2i {
    int x;
    int y;
} vec2i;

static inline vec2i vec2i_add(vec2i a, vec2i b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

static inline vec2i vec2i_sub(vec2i a, vec2i b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

static inline vec2i vec2i_mult(vec2i a, vec2i b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

static inline vec2f vec2f_add(vec2f a, vec2f b) {
    a.fx += b.fx;
    a.fy += b.fy;
    return a;
}

static inline vec2f vec2f_sub(vec2f a, vec2f b) {
    a.fx -= b.fx;
    a.fy -= b.fy;
    return a;
}

static inline vec2f vec2f_mult(vec2f a, vec2f b) {
    a.fx = fixedpt_xmul(a.fx, b.fx);
    a.fy = fixedpt_xmul(a.fy, b.fy);
    return a;
}

static inline vec2f vec2f_div(vec2f a, fixedpt div) {
    a.fx = fixedpt_xdiv(a.fx, div);
    a.fy = fixedpt_xdiv(a.fy, div);
    return a;
}

static inline vec2i vec2f_to_i(vec2f f) {
    vec2i i;
    i.x = fixedpt_toint(f.fx);
    i.y = fixedpt_toint(f.fy);
    return i;
}

static inline vec2f vec2i_to_f(vec2i i) {
    vec2f f;
    f.fx = fixedpt_fromint(i.x);
    f.fy = fixedpt_fromint(i.y);
    return f;
}

static inline fixedpt vec2f_magsqr(vec2f a) {
    return fixedpt_xmul(a.fx, a.fx) + fixedpt_xmul(a.fy, a.fy);
}

static inline fixedpt vec2f_mag(vec2f a) {
    return fixedpt_sqrt(fixedpt_xmul(a.fx, a.fx) + fixedpt_xmul(a.fy, a.fy));
}

static inline vec2f vec2f_norm(vec2f a) {
    fixedpt mag = vec2f_mag(a);
    a.fx = fixedpt_xdiv(a.fx, mag);
    a.fy = fixedpt_xdiv(a.fy, mag);
    return a;
}

static inline fixedpt vec2f_distsqr(vec2f a, vec2f b) {
    return vec2f_magsqr(vec2f_sub(a, b));
}
static inline fixedpt vec2f_dist(vec2f a, vec2f b) {
    return vec2f_mag(vec2f_sub(a, b));
}
static inline fixedpt vec2f_manhattan_dist(vec2f a, vec2f b) {
    return fixedpt_abs(a.fx - b.fx) + fixedpt_abs(a.fy - b.fy);
}

static inline vec2i vec2i_create(int x, int y) {
    vec2i v;
    v.x = x;
    v.y = y;
    return v;
}

static inline vec2f vec2f_create(fixedpt x, fixedpt y) {
    vec2f v;
    v.fx = x;
    v.fy = y;
    return v;
}

#endif // VEC_H
