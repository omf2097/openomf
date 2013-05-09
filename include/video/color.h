#ifndef _COLOR_H
#define _COLOR_H

#define COLOR_MENU_LINE   color_create(0,0,89,255)
#define COLOR_MENU_BORDER color_create(0,0,243,255)
#define COLOR_MENU_BG     color_create(4,4,16,210)

typedef struct color_t color;

struct color_t {
    unsigned char r,g,b,a;
};

color color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

#endif // _COLOR_H