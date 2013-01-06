#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("test_language <languagefile> <string #>\n");
        return 0;
    }
    
    sd_language *language = 0;
    if(sd_language_load(language, argv[1])) {
        printf("Language file could not be loaded!\n");
        return 1;
    }
    
    sd_language_delete(language);
    return 0;
}
