#include <math.h>
#include <stdlib.h>
#include "utils/miscmath.h"

int clamp(int val, int min, int max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

float clampf(float val, float min, float max) {
    if(val > max) return max;
    if(val < min) return min;
    return val;
}

int max3(int a, int b, int c) {
    int max = a;
    if(b > max) max = b;
    if(c > max) max = c;
    return max;
}

int max2(int a, int b) { return (a > b) ? a : b; }
int min2(int a, int b) { return (a > b) ? b : a; }

float dist(float a, float b) {
    return fabsf((a < b ? a : b) - (a > b ? a : b)) * (a < b ? 1 : -1);
}
