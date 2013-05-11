#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("test_language <languagefile> <string #>\n");
        printf("If string# is -1, then ALL strings will be printed.\n");
        return 0;
    }
    
    sd_language *language = sd_language_create();
    if(sd_language_load(language, argv[1])) {
        printf("Language file could not be loaded!\n");
        return 1;
    }
    
    int id = atoi(argv[2]);
    if(id < 0) {
        for(int i = 0; i < language->count; i++) {
            printf("#%d => %s\n", i, language->strings[i].description);
            printf("%s\n", language->strings[i].data);
        }
    } else if(id >= 0 && id < language->count) {
        printf("Title: %s\n", language->strings[id].description);
        printf("Data: %s\n", language->strings[id].data);
    } else {
        printf("String not found!\n");
    }
    
    sd_language_delete(language);
    return 0;
}
