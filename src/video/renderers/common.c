#include "video/renderers/common.h"
#include "utils/miscmath.h"

void find_resolution_for_aspect_ratio(SDL_Rect *view, const SDL_Rect *window, unsigned x_ratio, unsigned y_ratio) {
    float rx = (float)x_ratio / y_ratio;
    float ry = (float)y_ratio / x_ratio;
    view->w = min2(window->w, window->h * rx);
    view->h = view->w * ry;
    view->x = (window->w - view->w) / 2;
    view->y = (window->h - view->h) / 2;
}
