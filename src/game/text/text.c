#include "game/text/text.h"
#include "video/video.h"
#include "utils/log.h"
#include <shadowdive/shadowdive.h>
#include <string.h>
#include <stdlib.h>

font font_small;
font font_large;

void font_create(font *font) {
    font->size = FONT_UNDEFINED;
    vector_create(&font->textures, sizeof(texture*));
}

void font_free(font *font) {
    font->size = FONT_UNDEFINED;
    iterator it;
    vector_iter_begin(&font->textures, &it);
    texture **tex = NULL;
    while((tex = iter_next(&it)) != NULL) {
        texture_free(*tex);
        free(*tex);
    }
    vector_free(&font->textures);
}

int font_load(font *font, const char* filename, unsigned int size) {
    sd_rgba_image *img;
    sd_font *sdfont;
    int pixsize;
    texture *tex;
    
    // Find vertical size
    switch(size) {
        case FONT_BIG: pixsize = 8; break;
        case FONT_SMALL: pixsize = 6; break;
        default:
            return 1;
    }
    
    // Open font file
    sdfont = sd_font_create();
    if(sd_font_load(sdfont, filename, pixsize)) {
        sd_font_delete(sdfont);
        return 2;
    }
    
    // Load into textures
    img = sd_rgba_image_create(pixsize, pixsize);
    for(int i = 0; i < 224; i++) {
        tex = malloc(sizeof(texture));
        sd_font_decode(sdfont, img, i, 0xFF, 0xFF, 0xFF);
        texture_create(tex, img->data, img->w, img->h);
        vector_append(&font->textures, &tex);
    }
    
    // Set font info vars
    font->w = pixsize;
    font->h = pixsize;
    font->size = size;
    
    // Free resources
    sd_rgba_image_delete(img);
    sd_font_delete(sdfont);
    return 0;
}

int fonts_init() {
    font_create(&font_small);
    font_create(&font_large);
    if (font_load(&font_small, "resources/CHARSMAL.DAT", FONT_SMALL)) {
        PERROR("Unable to load resources/CHARSMAL.DAT");
        return 1;
    }
    if (font_load(&font_large, "resources/GRAPHCHR.DAT", FONT_BIG)) {
        PERROR("Unable to load resources/GRAPHCHR.DAT");
        return 1;
    }
    return 0;
}

void fonts_close() {
    font_free(&font_small);
    font_free(&font_large);
}

void font_render_len(font *font, const char *text, int len, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    int pos_x = x;
    texture **tex = NULL;
    for(int i = 0; i < len; i++) {
        int code = text[i] - 32;
        tex = vector_get(&font->textures, code);
        video_render_char(*tex, pos_x, y, r, g, b);
        pos_x += font->w;
    }
}

void font_render(font *font, const char *text, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    int len = strlen(text);
    font_render_len(font, text, len, x, y, r, g, b);
}

void font_render_wrapped(font *font, const char *text, int x, int y, int w, unsigned char r, unsigned char g, unsigned char b) {
    int len = strlen(text);
    if (font->w*len < w) {
        // short enough text that we don't need to wrap

        // render it centered, at least for now
        int xoff = (w - font->w*len)/2;
        font_render_len(font, text, len, x + xoff, y, r, g, b);
    } else {
        // ok, we actually have to do some real work
        // look ma, no mallocs!
        char *end = strchr(text, '\0');
        const char *start = text;
        const char *stop = start;
        const char *tmpstop;
        int maxlen = w/font->w;
        int yoff = 0;

        while (start != end) {
            while(1) {
                tmpstop = strchr(stop+1, ' ');
                if (tmpstop == NULL) {
                    stop = end-1;
                    break;
                } else if ((tmpstop - start) > maxlen) {
                    break;
                } else {
                    stop = tmpstop;
                }
            }
            int len = stop - start;
            int xoff = (w - font->w*len)/2;
            font_render_len(font, start, len, x + xoff, y + yoff, r, g, b);
            yoff += font->h+1;
            start = stop+1;
            stop = start;
        }
    }
}
