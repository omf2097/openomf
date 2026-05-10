#ifndef OKLAB_H
#define OKLAB_H

/* C port of OKLab from https://bottosson.github.io/posts/oklab/
 *
 * Copyright (c) 2020 Björn Ottosson
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "utils/miscmath.h"
#include <math.h>
#include <stdint.h>

typedef struct oklab_color {
    double L; // perceived lightness
    double a; // how green/red the color is
    double b; // how blue/yellow the color is
} oklab_color;

static inline double srgb_to_linear_rgb(uint8_t v) {
    double c = v / 255.0;
    if(c <= 0.04045) {
        return c / 12.92;
    }
    return pow((c + 0.055) / 1.055, 2.4);
}

static inline uint8_t linear_rgb_to_srgb(double c) {
    if(c <= 0.003130804) {
        c *= 12.92;
    } else {
        c = 1.055 * pow(c, 1.0 / 2.4) - 0.055;
    }
    int v = (int)(c * 255.0 + 0.5);
    return (uint8_t)(clamp(v, 0, 255));
}

static inline oklab_color rgb_to_oklab(uint8_t r, uint8_t g, uint8_t b) {
    double lr = srgb_to_linear_rgb(r);
    double lg = srgb_to_linear_rgb(g);
    double lb = srgb_to_linear_rgb(b);
    double l = cbrt(0.4122214708 * lr + 0.5363325363 * lg + 0.0514459929 * lb);
    double m = cbrt(0.2119034982 * lr + 0.6806225601 * lg + 0.1074740658 * lb);
    double s = cbrt(0.0883024619 * lr + 0.2817188376 * lg + 0.6299787005 * lb);
    oklab_color out;
    out.L = 0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s;
    out.a = 1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s;
    out.b = 0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s;
    return out;
}

// find the relative distance between 2 oklab colors
static inline double oklab_dist_sq(oklab_color a, oklab_color b) {
    double dL = a.L - b.L;
    double da = a.a - b.a;
    double db = a.b - b.b;
    return dL * dL + da * da + db * db;
}

static inline void oklab_to_rgb(oklab_color lab, uint8_t *r, uint8_t *g, uint8_t *b) {
    double l = pow(lab.L + 0.3963377774 * lab.a + 0.2158037573 * lab.b, 3.0);
    double m = pow(lab.L - 0.1055613458 * lab.a - 0.0638541728 * lab.b, 3.0);
    double s = pow(lab.L - 0.0894841775 * lab.a - 1.2914855480 * lab.b, 3.0);
    double lr = +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    double lg = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    double lb = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;
    *r = linear_rgb_to_srgb(lr);
    *g = linear_rgb_to_srgb(lg);
    *b = linear_rgb_to_srgb(lb);
}

#endif // OKLAB_H
