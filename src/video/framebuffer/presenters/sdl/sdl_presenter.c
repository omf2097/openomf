#include "video/framebuffer/presenters/sdl/sdl_presenter.h"

#include "utils/log.h"

#include <SDL.h>
#include <limits.h>
#include <stdlib.h>

typedef struct sdl_presenter {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int texture_w;
    int texture_h;
    bool video_initialized;
} sdl_presenter;

static const char *get_name(void) {
    return "SDL2";
}

static void close_context(void *userdata) {
    sdl_presenter *ctx = userdata;
    if(ctx->texture) {
        SDL_DestroyTexture(ctx->texture);
        ctx->texture = NULL;
    }
    if(ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
        ctx->renderer = NULL;
    }
    if(ctx->window) {
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
    }
    ctx->texture_w = 0;
    ctx->texture_h = 0;
    if(ctx->video_initialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        ctx->video_initialized = false;
    }
}

static bool open_context(void *userdata, int width, int height, bool fullscreen, bool vsync, int scaling_mode) {
    sdl_presenter *ctx = userdata;
    if(width <= 0 || height <= 0 || ctx->video_initialized || SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        log_error("SDL framebuffer video initialization failed: %s", SDL_GetError());
        return false;
    }
    ctx->video_initialized = true;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaling_mode == 1 ? "1" : "0");
    ctx->window = SDL_CreateWindow("OpenOMF", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
                                   SDL_WINDOW_SHOWN | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
    if(!ctx->window) {
        log_error("SDL framebuffer window: %s", SDL_GetError());
        goto fail;
    }
    ctx->renderer =
        SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED | (vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if(!ctx->renderer) {
        ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if(!ctx->renderer) {
        log_error("SDL framebuffer renderer: %s", SDL_GetError());
        goto fail;
    }
    return true;

fail:
    close_context(ctx);
    return false;
}

static bool present_frame(void *userdata, const framebuffer_image *image, const irect *destination) {
    sdl_presenter *ctx = userdata;
    if(!ctx->renderer || !framebuffer_image_valid(image) || !destination || destination->w <= 0 ||
       destination->h <= 0 || image->pitch > INT_MAX) {
        return false;
    }

    if(!ctx->texture || ctx->texture_w != image->width || ctx->texture_h != image->height) {
        if(ctx->texture) {
            SDL_DestroyTexture(ctx->texture);
            ctx->texture = NULL;
        }
        ctx->texture_w = image->width;
        ctx->texture_h = image->height;
        ctx->texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING,
                                         image->width, image->height);
        if(!ctx->texture) {
            log_error("SDL framebuffer texture: %s", SDL_GetError());
            return false;
        }
    }
    const SDL_Rect dst = {destination->x, destination->y, destination->w, destination->h};
    if(SDL_UpdateTexture(ctx->texture, NULL, image->pixels, (int)image->pitch) != 0 ||
       SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255) != 0 || SDL_RenderClear(ctx->renderer) != 0 ||
       SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, &dst) != 0) {
        log_error("SDL framebuffer presentation failed: %s", SDL_GetError());
        return false;
    }
    SDL_RenderPresent(ctx->renderer);
    return true;
}

static void destroy_context(void *userdata) {
    sdl_presenter *ctx = userdata;
    if(ctx) {
        close_context(ctx);
        free(ctx);
    }
}

bool sdl_framebuffer_presenter_create(framebuffer_presenter *presenter) {
    if(!presenter) {
        return false;
    }
    sdl_presenter *ctx = calloc(1, sizeof(*ctx));
    if(!ctx) {
        return false;
    }
    presenter->ctx = ctx;
    presenter->get_name = get_name;
    presenter->open = open_context;
    presenter->present = present_frame;
    presenter->close = close_context;
    presenter->destroy = destroy_context;
    return true;
}
