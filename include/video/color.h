#ifndef _COLOR_H
#define _COLOR_H

typedef struct color_t color;

struct color_t {
    unsigned char r,g,b,a;
};

color color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif // _COLOR_H