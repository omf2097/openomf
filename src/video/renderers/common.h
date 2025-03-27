#ifndef VIDEO_COMMON_H
#define VIDEO_COMMON_H

#include <SDL_rect.h>

void find_resolution_for_aspect_ratio(SDL_Rect *view, const SDL_Rect *window, unsigned ratio_w, unsigned ratio_h);

#endif // VIDEO_COMMON_H
