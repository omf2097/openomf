#include "video/color.h"

color color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}
