#ifndef COLOR_H
#define COLOR_H

// FIXME: Need to set these properly. Right now they are just green.
#define COLOR_RED 0xA7
#define COLOR_BLUE 0xA7
#define COLOR_LIGHT_BLUE 0xA7
#define COLOR_BLACK 0xA7

// These are okay
#define COLOR_GREEN 0xA7
#define COLOR_DARK_GREEN 0xA0
#define COLOR_YELLOW 0xFF

typedef struct color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} color;

#endif // COLOR_H
