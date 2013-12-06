#include <shadowdive/shadowdive.h>
#include <stdio.h>
#include <string.h>

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

void print_bytes(char *buf, int len, int line, int padding) {
    for(int k = 0; k < padding; k++) {
        printf(" ");
    }
    for(int i = 1; i <= len; i++) {
        printf("%02x ", (uint8_t)buf[i]);
        if(i % line == 0) {
            printf("\n");
            for(int k = 0; k < padding; k++) {
                printf(" ");
            }
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("test_tourn <tournamentfile> [-d logodump.ppm]\n");
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
    
    // Open tournament file
    sd_tournament_file *trn = sd_tournament_create();
    int ret = sd_tournament_load(trn, argv[1]);
    if(ret == SD_FILE_OPEN_ERROR) {
        printf("Tournament file could not be loaded!\n");
        return 1;
    } else if(ret == SD_FILE_PARSE_ERROR) {
        printf("Invalid file format!\n");
        return 1;
    }

    // Print enemy data
    printf("Enemies:\n");
    for(int i = 0; i < trn->enemy_count; i++) {
        printf("[%d] %s:\n", i, trn->enemies[i]->name);
        printf("  - Wins:        %d\n", trn->enemies[i]->wins);
        printf("  - Losses:      %d\n", trn->enemies[i]->losses);
        printf("  - Robot ID:    %d\n", trn->enemies[i]->robot_id);
        printf("  - Offense:     %d\n", trn->enemies[i]->offense);
        printf("  - Defense:     %d\n", trn->enemies[i]->defense);
        printf("  - Money:       %d\n", trn->enemies[i]->money);
        printf("  - Color:       %d,%d,%d\n", 
            trn->enemies[i]->color_1,
            trn->enemies[i]->color_2,
            trn->enemies[i]->color_3);
        printf("  - Stats:       ");
        print_bytes(trn->enemies[i]->stats, 8, 10, 0);
        printf("\n");
        printf("  - unk_block_a:\n");
        print_bytes(trn->enemies[i]->unk_block_a, 107, 16, 5);
        printf("\n");
        printf("  - Force arena: %d\n", trn->enemies[i]->force_arena);
        printf("  - unk_block_b: ");
        print_bytes(trn->enemies[i]->unk_block_a, 3, 16, 0);
        printf("\n");
        printf("  - Movement:    %d\n", trn->enemies[i]->movement);
        printf("  - unk_block_c: ");
        print_bytes(trn->enemies[i]->unk_block_c, 6, 16, 0);
        printf("\n");
        printf("  - Enhancement: ");
        print_bytes(trn->enemies[i]->enhancements, 11, 16, 0);
        printf("\n");
        printf("  - Flags:       %d\n", trn->enemies[i]->flags);

        printf("\n");
        for(int k = 0; k < MAX_TRN_LOCALES; k++) {
            if(trn->enemies[i]->quote[k] == NULL) continue;
            printf("  - %s quote: %s\n", 
                language_names[k], 
                trn->enemies[i]->quote[k]);
        }
    }
    
    printf("\nTournament details:\n");
    printf("Enemy count         : %d\n", trn->enemy_count);
    printf("BK name             : %s\n", trn->bk_name);
    printf("Winnings multiplier : %f\n", trn->winnings_multiplier);
    printf("Registration fee    : %d\n", trn->registration_free);
    printf("Initial value       : %d\n", trn->assumed_initial_value);
    printf("ID                  : %d\n", trn->tournament_id);
    printf("PIC file            : %s\n", trn->pic_file);

    printf("\nLocales:\n");
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        // Make sure the locale is valid
        if(trn->locales[i]->title[0] == 0) continue;

        // Print locale information
        printf("\n[%d] Locale '%s':\n", i, language_names[i]);
        printf("  - Logo: length = %d, size = (%d,%d), pos = (%d,%d)\n",
            trn->locales[i]->logo->img->len,
            trn->locales[i]->logo->img->w, 
            trn->locales[i]->logo->img->h,
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


    // See if logo dump was requested, and do so if it was
    if(dumplogo) {
        printf("\nDumping logo to '%s'.\n", dumpfile);
        sd_rgba_image *out = sd_sprite_image_decode(
            trn->locales[0]->logo->img, &trn->pal, -1);
        sd_rgba_image_to_ppm(out, dumpfile);
        sd_rgba_image_delete(out);
    }

    sd_tournament_delete(trn);
    return 0;
}
