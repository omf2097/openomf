#if ARGTABLE2_FOUND
#include <argtable2.h>
#elif ARGTABLE3_FOUND
#include <argtable3.h>
#endif
#include "formats/error.h"
#include "formats/pcx.h"
#include "utils/allocator.h"
#include <SDL.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#if _WIN32
#include <errno.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include "formats/vga_image.h"

static void show_pcx(pcx_file *pcx) {
    SDL_Surface *surface;
    SDL_Texture *background;
    SDL_Texture *rendertarget;
    SDL_Window *window = SDL_CreateWindow("OMF2097 Remake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 200,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if(!window) {
        printf("Could not create window: %s\n", SDL_GetError());
        return;
    }

    sd_rgba_image img;
    sd_vga_image_decode(&img, &pcx->image, &pcx->palette);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Rect dstrect;

    int scale = 1;

    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 320 * scale;
    dstrect.h = 200 * scale;

    uint32_t rmask, gmask, bmask, amask;

    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    if(!(surface = SDL_CreateRGBSurfaceFrom((void *)img.data, 320, 200, 32, 320 * 4, rmask, gmask, bmask, amask))) {
        printf("Could not create surface: %s\n", SDL_GetError());
        return;
    }

    if((background = SDL_CreateTextureFromSurface(renderer, surface)) == 0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    if((rendertarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 200)) ==
       0) {
        printf("Could not create texture: %s\n", SDL_GetError());
        return;
    }

    SDL_FreeSurface(surface);
    sd_rgba_image_free(&img);

    while(1) {
        SDL_Event e;
        if(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                break;
            }
        }
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, rendertarget);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderCopy(renderer, rendertarget, NULL, &dstrect);
        SDL_RenderPresent(renderer);
        SDL_Delay(10); // don't chew too much CPU
    }
}

static int file_exists(char const *filename) {
#if _WIN32
    return _access(filename, 0) == 0;
#else
    return access(filename, F_OK) == 0;
#endif
}

int main(int argc, char *argv[]) {
    struct arg_lit *help = arg_lit0("h", "help", "Print this help and exit");
    struct arg_file *file = arg_file0("f", "file", "<file>", "Input .PCX file");
    struct arg_lit *font = arg_lit0("F", "font", "Parse this file as a font");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, file, font, end};
    const char *progname = "pcxtool";

    if(arg_nullcheck(argtable) != 0) {
        printf("%s: Insufficient memory\n", progname);
        goto exit_0;
    }

    int nerrors = arg_parse(argc, argv, argtable);

    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    if(file->count == 0) {
        printf("The --file argument is required\n");
        goto exit_0;
    } else if(!file_exists(file->filename[0])) {
        printf("File %s cannot be accessed\n", file->filename[0]);
        goto exit_0;
    }

    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    if(font->count) {
        pcx_font *pcx_font = omf_calloc(1, sizeof(*pcx_font));
        if(pcx_load_font(pcx_font, file->filename[0]) != SD_SUCCESS) {
            printf("Could not load %s: %s\n", file->filename[0], strerror(errno));
            pcx_font_free(pcx_font);
            omf_free(pcx_font);
            goto exit_0;
        }
        printf("loaded font!\n");
        for(int g = 0; g < pcx_font->glyph_count; g++) {
            printf("glyph %d dimensions: %d,%d, %dx%d -> %d,%d\n", g, pcx_font->glyphs[g].x, pcx_font->glyphs[g].y,
                   pcx_font->glyphs[g].width, pcx_font->glyph_height, pcx_font->glyphs[g].x + pcx_font->glyphs[g].width,
                   pcx_font->glyphs[g].y + pcx_font->glyph_height);
        }
    } else {
        pcx_file *pcx = omf_calloc(1, sizeof(*pcx));
        if(pcx_load(pcx, file->filename[0]) != SD_SUCCESS) {
            printf("Could not load %s: %s\n", file->filename[0], strerror(errno));
            goto exit_0;
        }
        printf("Manufacturer: %" PRIx8 "\n", pcx->manufacturer);
        printf("Version: %" PRIx8 "\n", pcx->version);
        printf("Encoding: %" PRIx8 "\n", pcx->encoding);
        printf("Bits per plane: %" PRIx8 "\n", pcx->bits_per_plane);

        printf("Window X Min: %" PRIu16 "\n", pcx->window_x_min);
        printf("Window Y Min: %" PRIu16 "\n", pcx->window_y_min);
        printf("Window X Max: %" PRIu16 "\n", pcx->window_x_max);
        printf("Window Y Max: %" PRIu16 "\n", pcx->window_y_max);

        printf("Horz DPI: %" PRIu16 "\n", pcx->horz_dpi);
        printf("Vert DPI: %" PRIu16 "\n", pcx->vert_dpi);

        for(int i = 0; i < 48; ++i) {
            printf("Header Palette %d: %" PRIu8 "\n", i, pcx->header_palette[i]);
        }
        printf("Reserved: %" PRIu8 "\n", pcx->reserved);
        printf("Color Planes: %" PRIu8 "\n", pcx->color_planes);

        printf("Bytes Per Plane Line: %" PRIu16 "\n", pcx->bytes_per_plane_line);
        printf("Palette Info: %" PRIu16 "\n", pcx->palette_info);

        printf("Hor Scr Size: %" PRIu16 "\n", pcx->hor_scr_size);
        printf("Ver Scr Size: %" PRIu16 "\n", pcx->ver_scr_size);

        SDL_Init(SDL_INIT_VIDEO);

        show_pcx(pcx);
        pcx_free(pcx);
        omf_free(pcx);
    }

exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
