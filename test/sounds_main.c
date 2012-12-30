#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char **argv) {
    if (argc <= 2) {
        printf("Usage %s <filename> <output dir>\n", argv[0]);
        return 1;
    }

    sd_sound_file *sf = sd_sounds_create();
    int ret = sd_sounds_load(sf, argv[1]);
    if(ret != 0) {
        char error[100];
        sd_get_error(error, ret);
        printf("Error: %s\n", error);
        return 1;
    }

    printf("Found a total of %d sounds.\n", sf->sound_count);
    int k = 0;
    char filename[300];
    for(int i = 0; i < sf->sound_count; i++) {
        if(sf->sounds[i]) {
            k++;
            memset(filename, 0, 300);
            sprintf(filename, "%s/sound_%d.snd", argv[2], i);
            printf("Audio sample at slot %d with size of %d bytes. Exporting to '%s'.\n", i, sf->sounds[i]->len, filename);
            sd_sound_to_au(sf->sounds[i], (const char*)filename);
        }
    }
    printf("Found a total of %d samples with actual audio.\n", k);

    sd_sounds_delete(sf);
    return 0;
}
