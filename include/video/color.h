#ifndef _COLOR_H
#define _COLOR_H

#define COLOR_RED color_create(255,0,0,255)
#define COLOR_GREEN color_create(0,255,0,255)
#define COLOR_BLUE color_create(0,0,255,255)
#define COLOR_YELLOW color_create(255,255,0,255)
#define COLOR_WHITE color_create(255,255,255,255)
#define COLOR_BLACK color_create(0,0,0,255)

typedef struct color_t color;

struct color_t {
    unsigned char r,g,b,a;
};

color color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif // _COLOR_H
