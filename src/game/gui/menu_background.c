#include "game/gui/menu_background.h"
#include "formats/transparent.h"
#include "video/image.h"

#define COLOR_MENU_LINE 252
#define COLOR_MENU_BORDER 251
#define COLOR_MENU_LINE2 172
#define COLOR_MENU_BORDER1 164
#define COLOR_MENU_BORDER2 162
#define COLOR_MENU_BG 0

void menu_transparent_bg_create(surface *s, int w, int h) {
    surface_create(s, w, h, BACKGROUND_TRANSPARENT_INDEX);
}

void menu_background_create(surface *s, int w, int h) {
    image img;
    image_create(&img, w, h);
    image_clear(&img, COLOR_MENU_BG);
    for(int x = 5; x < w; x += 8) {
        image_line(&img, x, 0, x, h - 1, COLOR_MENU_LINE);
    }
    for(int y = 5; y < h; y += 8) {
        image_line(&img, 0, y, w - 1, y, COLOR_MENU_LINE);
    }
    image_rect(&img, 0, 0, w - 1, h - 1, COLOR_MENU_BORDER);
    surface_create_from_image(s, &img, MENU_TRANSPARENT_INDEX);
    image_free(&img);
}

// the *other* style menu background, used on VS and MELEE
void menu_background2_create(surface *s, int w, int h) {
    image img;
    image_create(&img, w, h);
    image_clear(&img, COLOR_MENU_BG);
    for(int x = 5; x < w; x += 5) {
        image_line(&img, x, 0, x, h - 1, COLOR_MENU_LINE2);
    }
    for(int y = 4; y < h; y += 5) {
        image_line(&img, 0, y, w - 1, y, COLOR_MENU_LINE2);
    }
    image_rect(&img, 1, 1, w - 2, h - 2, COLOR_MENU_BORDER2);
    image_rect(&img, 0, 0, w - 2, h - 2, COLOR_MENU_BORDER1);
    surface_create_from_image(s, &img, MENU_TRANSPARENT_INDEX);
    image_free(&img);
}

// create a transparent background with only the borders
void menu_background_border_create(surface *s, int w, int h) {
    image img;
    image_create(&img, w, h);
    image_clear(&img, 0);
    image_rect(&img, 0, 0, w - 1, h - 1, COLOR_MENU_BORDER);
    surface_create_from_image(s, &img, MENU_TRANSPARENT_INDEX);
    image_free(&img);
}
