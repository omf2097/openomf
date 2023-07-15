#ifndef COLOR_H
#define COLOR_H

#define COLOR_GREEN 0xA7
#define COLOR_DARK_GREEN 0xA0
#define COLOR_YELLOW 0xFF

typedef struct color_t color;

struct color_t {
    unsigned char r, g, b, a;
};

color color_create(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

#endif // COLOR_H
