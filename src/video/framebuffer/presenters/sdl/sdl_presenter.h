#ifndef OMF_SDL_FRAMEBUFFER_PRESENTER_H
#define OMF_SDL_FRAMEBUFFER_PRESENTER_H

#include "video/framebuffer/presenter.h"

/* SDL is strictly a display backend: software compositing never calls it. */
bool sdl_framebuffer_presenter_create(framebuffer_presenter *presenter);

#endif // OMF_SDL_FRAMEBUFFER_PRESENTER_H
