#include "game/text/text.h"
#include "video/video.h"
#include <shadowdive/shadowdive.h>
#include <string.h>

#include "utils/log.h"

void font_create(font *font) {
    font->size = FONT_UNDEFINED;
    vector_create(&font->textures, sizeof(texture));
}

void font_free(font *font) {
    font->size = FONT_UNDEFINED;
    iterator it;
    vector_iter_begin(&font->textures, &it);
    texture *tex;
    while((tex = iter_next(&it)) != NULL) {
        texture_free(tex);
    }
    vector_free(&font->textures);
}

int font_load(font *font, const char* filename, unsigned int size) {
    sd_rgba_image *img;
    sd_font *sdfont;
    int pixsize;
    texture tex;
    
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
    img = sd_rgba_image_create(8, pixsize);
    for(int i = 0; i < 224; i++) {
        sd_font_decode(sdfont, img, i, 0xFF, 0xFF, 0xFF);
        texture_create(&tex, img->data, img->w, img->h);
        vector_append(&font->textures, &tex);
    }
    
    // Set font info vars
    font->w = 8;
    font->h = pixsize;
    font->size = size;
    
    // Free resources
    sd_rgba_image_delete(img);
    sd_font_delete(sdfont);
    return 0;
}

void font_render(font *font, const char *text, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    int len = strlen(text);
    int pos_x = x;
    texture *tex = NULL;
    for(int i = 0; i < len; i++) {
        int code = text[i] - 32;
        tex = vector_get(&font->textures, code);
        video_render_char(tex, pos_x, y, r, g, b);
        pos_x += tex->w;
    }
}
