#ifndef OKLAB_H
#define OKLAB_H

#include <math.h>
#include <stdint.h>

typedef struct oklab_color {
    double L;
    double a;
    double b;
} oklab_color;

static inline double srgb_to_linear(uint8_t v) {
    double c = v / 255.0;
    if(c <= 0.04045) return c / 12.92;
    return pow((c + 0.055) / 1.055, 2.4);
}

static inline uint8_t linear_to_srgb(double c) {
    if(c <= 0.0031308) c = c * 12.92;
    else c = 1.055 * pow(c, 1.0 / 2.4) - 0.055;
    int v = (int)(c * 255.0 + 0.5);
    if(v < 0) return 0;
    if(v > 255) return 255;
    return (uint8_t)v;
}

static inline oklab_color rgb_to_oklab(uint8_t r, uint8_t g, uint8_t b) {
    double lr = srgb_to_linear(r), lg = srgb_to_linear(g), lb = srgb_to_linear(b);
    double l_ = 0.4122214708*lr + 0.5363325363*lg + 0.0514459929*lb;
    double m_ = 0.2119034982*lr + 0.6806225601*lg + 0.1074740658*lb;
    double s_ = 0.0883024619*lr + 0.2817188376*lg + 0.6299787005*lb;
    double l = cbrt(l_), m = cbrt(m_), s = cbrt(s_);
    oklab_color out;
    out.L = 0.2104542553*l + 0.7936177850*m - 0.0040720468*s;
    out.a = 1.9779984951*l - 2.4285922050*m + 0.4505937099*s;
    out.b = 0.0259040371*l + 0.7827717662*m - 0.8086757660*s;
    return out;
}

static inline double oklab_dist_sq(oklab_color a, oklab_color b) {
    double dL = a.L - b.L, da = a.a - b.a, db = a.b - b.b;
    return dL*dL + da*da + db*db;
}

static inline void oklab_to_rgb(oklab_color lab, uint8_t *r, uint8_t *g, uint8_t *b) {
    double l_ = lab.L + 0.3963377774*lab.a + 0.2158037573*lab.b;
    double m_ = lab.L - 0.1055613458*lab.a - 0.0638541728*lab.b;
    double s_ = lab.L - 0.0894841775*lab.a - 1.2914855480*lab.b;
    double l = l_*l_*l_, m = m_*m_*m_, s = s_*s_*s_;
    double lr = +4.0767416621*l - 3.3077115913*m + 0.2309699292*s;
    double lg = -1.2684380046*l + 2.6097574011*m - 0.3413193965*s;
    double lb = -0.0041960863*l - 0.7034186147*m + 1.7076147010*s;
    *r = linear_to_srgb(lr);
    *g = linear_to_srgb(lg);
    *b = linear_to_srgb(lb);
}

#endif // OKLAB_H