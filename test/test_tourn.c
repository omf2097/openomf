#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

const char *language_names[] = {
    "English",
    "German",
    "French",
    "Spanish",
    "Mexican",
    "Italian",
    "Polish",
    "Russian",
    "Undef",
    "Undef"
};

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("test_tourn <tournamentfile> [-d logodump.ppm|-o output.trn]\n");
        return 0;
    }

    // Get logodump path
    int dumplogo = 0;
    char *dumpfile = 0;
    if(argc == 4) {
        if(strcmp(argv[2], "-d") == 0) {
            dumplogo = 1;
            dumpfile = (char*)argv[3];
        }
    }

    int dumptourn = 0;
    if(argc == 4) {
        if(strcmp(argv[2], "-o") == 0) {
            dumptourn = 1;
            dumpfile = (char*)argv[3];
        }
    }
    
    // Open tournament file
    sd_tournament_file *trn = malloc(sizeof(sd_tournament_file));
    sd_tournament_create(trn);
    int ret = sd_tournament_load(trn, argv[1]);
    if(ret != SD_SUCCESS) {
        printf("Shadowdive error %d: %s\n", ret, sd_get_error(ret));
        return 1;
    }

    // Print enemy data
    printf("Enemies:\n");
    for(int i = 0; i < trn->enemy_count; i++) {
        print_pilot_info(trn->enemies[i]);

        for(int k = 0; k < MAX_TRN_LOCALES; k++) {
            if(trn->quotes[i][k] == NULL) continue;
            printf("  - %s quote: %s\n", 
                language_names[k], 
                trn->quotes[i][k]);
        }

        printf("\n");
    }

    printf("\nLocales:\n");
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        // Make sure the locale is valid
        if(trn->locales[i]->title[0] == 0) continue;

        // Print locale information
        printf("\n[%d] Locale '%s':\n", i, language_names[i]);
        printf("  - Logo: length = %d, size = (%d,%d), pos = (%d,%d)\n",
            trn->locales[i]->logo->len,
            trn->locales[i]->logo->width, 
            trn->locales[i]->logo->height,
            trn->locales[i]->logo->pos_x, 
            trn->locales[i]->logo->pos_y);
        printf("  - Title: %s\n", trn->locales[i]->title);

        // Print victory text pages
        printf("  - Text pages:\n");
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                if(trn->locales[i]->end_texts[har][page] != NULL
                    && trn->locales[i]->end_texts[har][page][0] != 0) 
                {
                    printf("    * Page (%d,%d): %s\n", 
                        har, 
                        page, 
                        trn->locales[i]->end_texts[har][page]);
                }
                
            }
        }

        // Description
        printf("  - Description: %s\n", trn->locales[i]->description);
    }

    printf("\nTournament details:\n");
    printf("  - Enemy count         : %d\n", trn->enemy_count);
    printf("  - BK name             : %s\n", trn->bk_name);
    printf("  - Winnings multiplier : %f\n", trn->winnings_multiplier);
    printf("  - Unknown             : %d\n", trn->unknown_a);
    printf("  - Registration fee    : %d\n", trn->registration_free);
    printf("  - Initial value       : %d\n", trn->assumed_initial_value);
    printf("  - ID                  : %d\n", trn->tournament_id);
    printf("  - PIC file            : %s\n", trn->pic_file);

    // See if logo dump was requested, and do so if it was
    if(dumplogo) {
        printf("\nDumping logo to '%s'.\n", dumpfile);
        sd_rgba_image out;
        sd_sprite_rgba_decode(
            &out,
            trn->locales[0]->logo,
            &trn->pal,
            -1);
        sd_rgba_image_to_ppm(&out, dumpfile);
        sd_rgba_image_free(&out);
    }
    if(dumptourn) {
        if(sd_tournament_save(trn, dumpfile) != SD_SUCCESS) {
            printf("Dumping tournament file failed.\n");
        }
    }

    sd_tournament_free(trn);
    free(trn);
    return 0;
}
