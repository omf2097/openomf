#include "utils/miscmath.h"
#include <limits.h>
#include <math.h>

int clamp(int val, int min, int max) {
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

float clampf(float val, float min, float max) {
    if(val > max)
        return max;
    if(val < min)
        return min;
    return val;
}

int clamp_long_to_int(long val) {
    if(val > INT_MAX)
        return INT_MAX;
    if(val < INT_MIN)
        return INT_MIN;
    return val;
}

int max3(int a, int b, int c) {
    int max = a;
    if(b > max)
        max = b;
    if(c > max)
        max = c;
    return max;
}

int max2(int a, int b) {
    return (a > b) ? a : b;
}

int min2(int a, int b) {
    return (a > b) ? b : a;
}

unsigned umax2(unsigned a, unsigned b) {
    return (a > b) ? a : b;
}

unsigned umin2(unsigned a, unsigned b) {
    return (a > b) ? b : a;
}

unsigned udist(unsigned a, unsigned b) {
    return (a > b) ? a - b : b - a;
}

float dist(float a, float b) {
    return fabsf((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1.0f : -1.0f);
}

unsigned powu(unsigned x, unsigned y) {
    unsigned ret = 1;
    while(y--) {
        ret *= x;
    }
    return ret;
}
