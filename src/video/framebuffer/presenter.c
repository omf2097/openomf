#include "video/framebuffer/presenter.h"

#include <stdint.h>
#include <string.h>

#ifdef ENABLE_SDL_FRAMEBUFFER_PRESENTER
#include "video/framebuffer/presenters/sdl/sdl_presenter.h"
#endif

typedef bool (*presenter_create_fn)(framebuffer_presenter *presenter);

typedef struct presenter_backend {
    const char *name;
    presenter_create_fn create;
} presenter_backend;

#ifdef ENABLE_SDL_FRAMEBUFFER_PRESENTER
static const presenter_backend backends[] = {
    {"SDL2", sdl_framebuffer_presenter_create},
};
#define PRESENTER_COUNT (int)(sizeof(backends) / sizeof(backends[0]))
#else
#define PRESENTER_COUNT 0
#endif

bool framebuffer_image_valid(const framebuffer_image *image) {
    if(!image || !image->pixels || image->format != FRAMEBUFFER_FORMAT_ABGR8888 || image->width <= 0 ||
       image->height <= 0) {
        return false;
    }

    if((size_t)image->width > SIZE_MAX / sizeof(uint32_t)) {
        return false;
    }
    const size_t row_bytes = (size_t)image->width * sizeof(uint32_t);
    return image->pitch >= row_bytes && (size_t)image->height <= SIZE_MAX / image->pitch;
}

int framebuffer_presenter_count(void) {
    return PRESENTER_COUNT;
}

const char *framebuffer_presenter_name(int index) {
#ifdef ENABLE_SDL_FRAMEBUFFER_PRESENTER
    if(index >= 0 && index < PRESENTER_COUNT) {
        return backends[index].name;
    }
#else
    (void)index;
#endif
    return NULL;
}

bool framebuffer_presenter_create(framebuffer_presenter *presenter, const char *preferred_name) {
    if(!presenter) {
        return false;
    }
    memset(presenter, 0, sizeof(*presenter));
#ifdef ENABLE_SDL_FRAMEBUFFER_PRESENTER
    for(int i = 0; i < PRESENTER_COUNT; i++) {
        if((!preferred_name || !preferred_name[0] || strcmp(backends[i].name, preferred_name) == 0) &&
           backends[i].create(presenter)) {
            return true;
        }
    }
#else
    (void)preferred_name;
#endif
    return false;
}

void framebuffer_presenter_destroy(framebuffer_presenter *presenter) {
    if(presenter) {
        if(presenter->destroy) {
            presenter->destroy(presenter->ctx);
        }
        memset(presenter, 0, sizeof(*presenter));
    }
}
