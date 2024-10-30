/** @file main.c
 * @brief .AF file editor tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/fonts.h"
#include <SDL2/SDL.h>
#if ARGTABLE2_FOUND
#include <argtable2.h>
#elif ARGTABLE3_FOUND
#include <argtable3.h>
#endif
#include <stdint.h>
#include <string.h>

SDL_Surface *render_text(sd_font *font, const char *text, int area_w) {
    // Vars
    unsigned int rmask, gmask, bmask, amask;
    int char_w, char_h;
    int pix_w, pix_h;
    int slen;
    SDL_Surface *surface;
    SDL_Surface *tmp;
    SDL_Rect dst;
    sd_rgba_image img;

    // Required surface size
    slen = strlen(text);
    char_h = (slen * font->h) / area_w + 1;
    pix_h = char_h * font->h;
    if(char_h == 1) {
        char_w = slen;
        pix_w = slen * font->h;
    } else {
        char_w = area_w / font->h;
        pix_w = area_w;
    }

    // Create an empty SDL surface
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
    if((surface = SDL_CreateRGBSurface(0, pix_w, pix_h, 32, rmask, gmask, bmask, amask)) == 0) {
        return 0;
    }
    if((tmp = SDL_CreateRGBSurface(0, font->h, font->h, 32, rmask, gmask, bmask, amask)) == 0) {
        SDL_FreeSurface(surface);
        return 0;
    }

    // Render text
    dst.w = font->h;
    dst.h = font->h;
    sd_rgba_image_create(&img, font->h, font->h);
    for(int i = 0; i < slen; i++) {
        sd_font_decode_rgb(font, &img, text[i] - 32, 64, 128, 64);
        memcpy(tmp->pixels, img.data, 4 * font->h * font->h);
        dst.y = i / char_w * font->h;
        dst.x = i % char_w * font->h;
        SDL_BlitSurface(tmp, 0, surface, &dst);
    }

    // All done.
    SDL_FreeSurface(tmp);
    sd_rgba_image_free(&img);
    return surface;
}

void export_to(sd_font *font, const char *filename, int split) {
    if(split) {
        char path[256];
        sd_rgba_image ch_img;
        sd_rgba_image_create(&ch_img, font->h, font->h);
        for(int i = 32; i < 256; i++) {
            sd_font_decode_rgb(font, &ch_img, (char)(i - 32), 0, 0, 0);
            sprintf(path, "%s/uni%04x.png", filename, i);
            int ret = sd_rgba_image_to_png(&ch_img, path);
            if(ret != SD_SUCCESS) {
                printf("Error while exporting to %s: %s.", path, sd_get_error(ret));
            }
        }
        sd_rgba_image_free(&ch_img);
    } else {
        sd_rgba_image ch_img;
        sd_rgba_image dst_img;
        sd_rgba_image_create(&dst_img, font->h * 16, font->h * 16);
        sd_rgba_image_create(&ch_img, font->h, font->h);
        for(int i = 32; i < 256; i++) {
            int x = i % 16;
            int y = i / 16;
            sd_font_decode_rgb(font, &ch_img, (char)(i - 32), 0, 0, 0);
            sd_rgba_image_blit(&dst_img, &ch_img, x * font->h, y * font->h);
        }
        sd_rgba_image_free(&ch_img);
        int ret = sd_rgba_image_to_png(&dst_img, filename);
        if(ret != SD_SUCCESS) {
            printf("Error while exporting to %s: %s.", filename, sd_get_error(ret));
        }
        sd_rgba_image_free(&dst_img);
    }
}

void display(sd_font *font, int _sc, const char *text) {
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Create surface for font rendering
    SDL_Surface *surface = 0;
    if((surface = render_text(font, text, 320)) == 0) {
        printf("Failed to render text!\n");
        goto d_exit_0;
    }

    // Init window
    SDL_Window *window = SDL_CreateWindow("Fonttool v0.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320 * _sc,
                                          200 * _sc, SDL_WINDOW_SHOWN);
    if(!window) {
        printf("Could not create window: %s\n", SDL_GetError());
        goto d_exit_1;
    }

    // Rects
    SDL_Rect srcrect, dstrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.w = surface->w;
    srcrect.h = surface->h;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = surface->w * _sc;
    dstrect.h = surface->h * _sc;

    // Run until windows closed.
    SDL_Surface *window_surface = SDL_GetWindowSurface(window);
    Uint32 tcolor = SDL_MapRGBA(window_surface->format, 0, 0, 0, 0);
    SDL_Event e;
    int run = 1;
    while(run) {
        if(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                run = 0;
            }
        }

        SDL_FillRect(window_surface, NULL, tcolor);
        if(_sc == 1) {
            SDL_BlitSurface(surface, &srcrect, window_surface, &dstrect);
        } else {
            SDL_BlitScaled(surface, &srcrect, window_surface, &dstrect);
        }
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    // Quit
    SDL_DestroyWindow(window);
d_exit_1:
    SDL_FreeSurface(surface);
d_exit_0:
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "font file");
    struct arg_int *fh = arg_int1("g", "height", "<value>", "font height in pixels (6 or 8)");
    struct arg_str *text = arg_str0("t", "text", "<value>", "text to show");
    struct arg_int *scale = arg_int0("s", "scale", "<value>", "scaling for the window");
    struct arg_file *export = arg_file0("e", "export", "<file>", "Export the full font to a PNG file");
    struct arg_lit *split = arg_lit0(NULL, "split", "Split to separate PNG files (set path in -e).");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, fh, text, scale, export, split, end};
    const char *progname = "fonttool";

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Font file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Requite either export or text
    if(export->count <= 0 && text->count <= 0) {
        printf("Use either --export or --text arguments!\n");
        goto exit_0;
    }

    // Font size
    int _fs = fh->ival[0];
    if(_fs < 6 || _fs > 8 || _fs == 7) {
        printf("Only valid values for fontsize are 6 and 8.\n");
        goto exit_0;
    }

    // Load fonts
    sd_font font;
    sd_font_create(&font);
    int ret = sd_font_load(&font, file->filename[0], _fs);
    if(ret != SD_SUCCESS) {
        printf("Couldn't load small font file! Error [%d] %s.\n", ret, sd_get_error(ret));
        goto exit_1;
    }

    // Export or display
    if(export->count > 0) {
        export_to(&font, export->filename[0], (split->count > 0));
    } else {
        // Scale
        int _sc = 1;
        if(scale->count > 0) {
            _sc = scale->ival[0];
        }
        if(_sc > 4)
            _sc = 4;
        if(_sc < 1)
            _sc = 1;

        display(&font, _sc, text->sval[0]);
    }

exit_1:
    sd_font_free(&font);
exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
