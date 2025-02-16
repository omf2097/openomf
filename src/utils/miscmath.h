#ifndef MISCMATH_H
#define MISCMATH_H

#include <limits.h>

#define MATH_PI 3.14159265f

static inline int clamp(int val, int min, int max) {
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

static inline float clampf(float val, float min, float max) {
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

static inline int clamp_long_to_int(long val) {
    if(val > INT_MAX)
        return INT_MAX;
    if(val < INT_MIN)
        return INT_MIN;
    return val;
}

static inline int max3(int a, int b, int c) {
    int max = a;
    if(b > max)
        max = b;
    if(c > max)
        max = c;
    return max;
}

static inline int max2(int a, int b) {
    return (a > b) ? a : b;
}

static inline int min2(int a, int b) {
    return (a > b) ? b : a;
}

static inline unsigned umax2(unsigned a, unsigned b) {
    return (a > b) ? a : b;
}

static inline unsigned umin2(unsigned a, unsigned b) {
    return (a > b) ? b : a;
}

static inline unsigned udist(unsigned a, unsigned b) {
    return (a > b) ? a - b : b - a;
}

static inline float dist(float a, float b) {
    return fabsf((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1.0f : -1.0f);
}

static inline size_t smin2(size_t a, size_t b) {
    return (a > b) ? b : a;
}

static inline unsigned powu(unsigned x, unsigned y) {
    unsigned ret = 1;
    while(y--) {
        ret *= x;
    }
    return ret;
}

#endif // MISCMATH_H
