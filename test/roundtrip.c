#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

int main(int argc, char **argv) {
    char buf[256];
    char *ext;

    if (argc < 3) {
        printf("Usage %s <input> <output>\n", argv[0]);
        return 1;
    }

    strncpy(buf, argv[1], 255);
    buf[255] = '0';
    basename(buf);

    ext = strrchr(buf, '.');
    if (ext == NULL) {
        printf("cannot determine file extension for %s\n", buf);
        return 1;
    }

    if (strncmp(ext, ".AF", 3) == 0) {
        sd_af_file *file = sd_af_create();
        printf("Loading AF file: %s\n", argv[1]);
        if (sd_af_load(file, argv[1]) == SD_SUCCESS) {
            printf("File loaded.\n");
        } else {
            printf("Unable to load file!\n");
            return 1;
        }
    } else if (strncmp(ext, ".BK", 3) == 0) {
        sd_bk_file *file = sd_bk_create();
        printf("Loading BK file: %s\n", argv[1]);
        if(sd_bk_load(file, argv[1]) == SD_SUCCESS) {
            printf("File loaded.\n");
            printf("Writing BK file to %s.\n", argv[2]);
            sd_bk_save(file, argv[2]);
            sd_bk_delete(file);
        } else {
            printf("Unable to load file!\n");
        }
    } else {
        printf("Unrecognized file extenion %s\n", ext);
        return 1;
    }
    return 0;
}


