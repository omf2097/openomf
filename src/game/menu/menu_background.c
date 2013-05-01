#include <stdlib.h>
#include "game/menu/menu_background.h"
#include "utils/log.h"

void draw_x_line(char *bitmap, int y, int x1, int x2, int rowspan, char r, char g, char b, char a) {
    int offset = rowspan*y*4;
    DEBUG("offset is %d", offset);
    for(int i = offset+(x1*4); i < offset+(x2*4); i+=4) {
        bitmap[i] = r;
        bitmap[i+1] = g;
        bitmap[i+2] = b;
        bitmap[i+3] = a;
    }
}

void draw_y_line(char *bitmap, int x, int y1, int y2, int rowspan, char r, char g, char b, char a) {
    int offset = (rowspan*y1*4)+(x*4);
    int stop = (rowspan*y2*4)+(x*4);
    for(int i = offset; i < stop; i+=rowspan*4) {
        bitmap[i] = r;
        bitmap[i+1] = g;
        bitmap[i+2] = b;
        bitmap[i+3] = a;
    }
}

void menu_background_create(texture *tex, int w, int h) {
    char bitmap[w*h*4];
    // the background is a 7x7 grid with bright lines around the border
    // the grid is centered on the middle of the right-hand side
    
    //first, fill in the background
    for(int i = 0; i < w*h*4; i+=4) {
        bitmap[i] = 4;
        bitmap[i+1] = 4;
        bitmap[i+2] = 16;
        bitmap[i+3] = 10;
    }

    // draw the grid
    // TODO this isn't centered as described above, which is how the game does it

    for(int i = 7; i < w; i+=8) {
        DEBUG("drawing %d/%d", i, w);
        draw_y_line(bitmap, i, 0, h-1, w, 0, 0, 35, 255);
    }
    for(int i = 7; i < h; i+=8) {
        draw_x_line(bitmap, i, 0, w, w, 0, 0, 35, 255);
    }

    // draw the border
    draw_x_line(bitmap, 0, 0, w, w, 0, 0, 243, 255); // top border
    draw_y_line(bitmap, 0, 0, h-1, w, 0, 0, 243, 255); // left border
    draw_x_line(bitmap, h-1, 0, w, w, 0, 0, 243, 255); // bottom border
    draw_y_line(bitmap, w-1, 0, h-1, w, 0, 0, 243, 255); // right border

    texture_create(tex, bitmap, w, h);
}

