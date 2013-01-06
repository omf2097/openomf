#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if(argc < 4) {
        printf("fonts <fontfile> <height> <character #>\n");
        printf("height: 6 for small fonts, 8 for big.");
        return 0;
    }
    
    // Char number
    int g = atoi(argv[3]);
    if(g < 0) g = 0;
    if(g >= 224) g = 223;
    
    // Font height
    int h = atoi(argv[2]);
    if(h < 6 || h > 8 || h == 7) {
        printf("Only values 6 and 8 allowed for height!\n");
        return 1;
    }

    sd_font *font = sd_font_create();
    if(sd_font_load(font, argv[1], h)) {
        printf("Font file couldn't be loaded!\n");
        sd_font_delete(font);
        return 1;
    }
    
    for(int i = 0; i < font->h; i++) {
        for(int k = 7; k >= 0; k--) {
            if(font->chars[g].data[i] & (1 << k)) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    sd_font_delete(font);
    return 0;
}
