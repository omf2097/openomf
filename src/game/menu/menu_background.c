#include <stdlib.h>
#include "game/menu/menu_background.h"
#include "utils/log.h"
#include "video/color.h"
#include "video/image.h"

#define COLOR_MENU_LINE   color_create(0,0,89,255)
#define COLOR_MENU_BORDER color_create(0,0,243,255)
#define COLOR_MENU_BG     color_create(4,4,16,210)

void menu_background_create(texture *tex, int w, int h) {
    image img;
    image_create(&img, w, h);
    image_clear(&img, COLOR_MENU_BG);
    for(int x = 7; x < w; x += 8) {
        image_line(&img, x, 0, x, h, COLOR_MENU_LINE);
    }
    for(int y = 7; y < h; y += 8) {
        image_line(&img, 0, y, w, y, COLOR_MENU_LINE);
    }
    image_rect(&img, 0, 0, w-1, h-1, COLOR_MENU_BORDER);
    texture_create_from_img(tex, &img);
    image_free(&img);
}

