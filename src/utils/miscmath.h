#ifndef MISCMATH_H
#define MISCMATH_H

#define MATH_PI 3.14159265f

int max3(int a, int b, int c);
float dist(float a, float b);
float clampf(float val, float _min, float _max);
int clamp(int val, int _min, int _max);
int max2(int a, int b);
int min2(int a, int b);
int clamp_long_to_int(long val);
unsigned powu(unsigned x, unsigned y);

#endif // MISCMATH_H
