#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if(argc < 4) {
        printf("fonts <fontfile> <type> <character #>\n");
        printf("type: 0 for small fonts, 1 for big.");
        return 0;
    }
    
    // Char number
    int n = atoi(argv[3]);
    if(n < 0) n = 0;
    if(n > 224) n = 224;

    if(argv[2][0] == '1') {
        sd_font_big *big = sd_font_big_create();
        int ret = sd_font_big_load(big, argv[1]);
        if(ret) {
            printf("Font file couldn't be loaded!\n");
            sd_font_big_delete(big);
            return 1;
        }
        
        for(int y = 0; y < 8; y++) {
            for(int x = 0; x < 8; x++) {
                if(big->chars[n].data[y] & (1 << x)) {
                    printf("#");
                } else {
                    printf(" ");
                }
            }
            printf("\n");
        }
        
        sd_font_big_delete(big);
    } else {
        sd_font_small *small = sd_font_small_create();
        int ret = sd_font_small_load(small, argv[1]);
        if(ret) {
            printf("Font file couldn't be loaded!\n");
            sd_font_small_delete(small);
            return 1;
        }
        
        
        sd_font_small_delete(small);
    }
    return 0;
}
