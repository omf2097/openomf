/** @file main.c
  * @brief TRN file parser tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../shared/pilot.h"

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

void print_locale(sd_tournament_locale *locale, int lang_id) {
    // Make sure the locale is valid
    if(locale->title[0] == 0)
        return;

    // Print locale information
    printf("\n[%d] Locale '%s':\n", lang_id, language_names[lang_id]);
    printf("  - Logo: length = %d, size = (%d,%d), pos = (%d,%d)\n",
        locale->logo->len,
        locale->logo->width,
        locale->logo->height,
        locale->logo->pos_x,
        locale->logo->pos_y);
    printf("  - Title: %s\n", locale->title);

    // Print victory text pages
    printf("  - Text pages:\n");
    for(int har = 0; har < 11; har++) {
        for(int page = 0; page < 10; page++) {
            if(locale->end_texts[har][page] != NULL
                && locale->end_texts[har][page][0] != 0)
            {
                printf("    * Page (%d,%d): %s\n",
                    har,
                    page,
                    locale->end_texts[har][page]);
            }
        }
    }

    // Description
    printf("  - Description: %s\n", locale->description);
}

void print_info(sd_tournament_file *trn) {
    printf("\nTournament details:\n");
    printf("  - Enemy count         : %d\n", trn->enemy_count);
    printf("  - BK name             : %s\n", trn->bk_name);
    printf("  - Winnings multiplier : %f\n", trn->winnings_multiplier);
    printf("  - Unknown             : %d\n", trn->unknown_a);
    printf("  - Registration fee    : %d\n", trn->registration_free);
    printf("  - Initial value       : %d\n", trn->assumed_initial_value);
    printf("  - ID                  : %d\n", trn->tournament_id);
    printf("  - PIC file            : %s\n", trn->pic_file);
}

int main(int argc, char *argv[]) {
   // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "TRN file");
    struct arg_int *locale = arg_int0(NULL, "locale", "<int>", "Only print locale");
    struct arg_int *pilot = arg_int0(NULL, "pilot", "<int>", "Only print pilot");
    struct arg_lit *info = arg_lit0(NULL, "info", "Only print tournament information");
    struct arg_file *output = arg_file0("o", "output", "<file>", "TRN output file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,locale,pilot,info,end};
    const char* progname = "trntool";

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 TRN file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Get tournament
    sd_tournament_file trn;
    sd_tournament_create(&trn);
    int ret = sd_tournament_load(&trn, file->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("TRN file %s could not be loaded: %s\n",
            file->filename[0],
            sd_get_error(ret));
        goto exit_0;
    }

    // Print requested stuff
    if(locale->count > 0) {
        int locale_id = locale->ival[0];
        if(locale_id < 0 || locale_id >= MAX_TRN_LOCALES) {
            printf("Locale ID out of bounds!\n");
            goto exit_1;
        }
        print_locale(trn.locales[locale_id], locale_id);
    } else if(pilot->count > 0) {
        int pilot_id = pilot->ival[0];
        print_pilot_info(trn.enemies[pilot_id]);
        for(int k = 0; k < MAX_TRN_LOCALES; k++) {
            if(trn.quotes[pilot_id][k] == NULL)
                continue;
            printf("  - %s quote: %s\n",
                language_names[k],
                trn.quotes[pilot_id][k]);
        }
        printf("\n");
    } else if(info->count > 0) {
        print_info(&trn);
    } else {
        printf("Enemies:\n");
        for(int i = 0; i < trn.enemy_count; i++) {
            print_pilot_info(trn.enemies[i]);
            for(int k = 0; k < MAX_TRN_LOCALES; k++) {
                if(trn.quotes[i][k] == NULL)
                    continue;
                printf("  - %s quote: %s\n",
                    language_names[k],
                    trn.quotes[i][k]);
            }
            printf("\n");
        }

        printf("\nLocales:\n");
        for(int i = 0; i < MAX_TRN_LOCALES; i++) {
            print_locale(trn.locales[i], i);
        }

        print_info(&trn);
    }

    // Output if asked
    if(output->count > 0) {
        ret = sd_tournament_save(&trn, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed to save TRN file to %s: %s\n",
                output->filename[0],
                sd_get_error(ret));
        }
    }

exit_1:
    sd_tournament_free(&trn);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}