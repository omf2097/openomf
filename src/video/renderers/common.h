#ifndef VIDEO_COMMON_H
#define VIDEO_COMMON_H

#include "utils/rect.h"

void find_resolution_for_aspect_ratio(irect *view, const irect *window, unsigned ratio_w, unsigned ratio_h);

#endif // VIDEO_COMMON_H
