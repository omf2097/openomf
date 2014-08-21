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

int main(int argc, char *argv[]) {
   // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "TRN file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "TRN output file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,end};
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
    
    // Just print all data for now.
    // We can add more flags later.

    printf("Enemies:\n");
    for(int i = 0; i < trn.enemy_count; i++) {
        print_pilot_info(trn.enemies[i]);

        for(int k = 0; k < MAX_TRN_LOCALES; k++) {
            if(trn.quotes[i][k] == NULL) continue;
            printf("  - %s quote: %s\n", 
                language_names[k], 
                trn.quotes[i][k]);
        }

        printf("\n");
    }

    printf("\nLocales:\n");
    for(int i = 0; i < MAX_TRN_LOCALES; i++) {
        // Make sure the locale is valid
        if(trn.locales[i]->title[0] == 0) continue;

        // Print locale information
        printf("\n[%d] Locale '%s':\n", i, language_names[i]);
        printf("  - Logo: length = %d, size = (%d,%d), pos = (%d,%d)\n",
            trn.locales[i]->logo->len,
            trn.locales[i]->logo->width, 
            trn.locales[i]->logo->height,
            trn.locales[i]->logo->pos_x, 
            trn.locales[i]->logo->pos_y);
        printf("  - Title: %s\n", trn.locales[i]->title);

        // Print victory text pages
        printf("  - Text pages:\n");
        for(int har = 0; har < 11; har++) {
            for(int page = 0; page < 10; page++) {
                if(trn.locales[i]->end_texts[har][page] != NULL
                    && trn.locales[i]->end_texts[har][page][0] != 0) 
                {
                    printf("    * Page (%d,%d): %s\n", 
                        har, 
                        page, 
                        trn.locales[i]->end_texts[har][page]);
                }
                
            }
        }

        // Description
        printf("  - Description: %s\n", trn.locales[i]->description);
    }

    printf("\nTournament details:\n");
    printf("  - Enemy count         : %d\n", trn.enemy_count);
    printf("  - BK name             : %s\n", trn.bk_name);
    printf("  - Winnings multiplier : %f\n", trn.winnings_multiplier);
    printf("  - Unknown             : %d\n", trn.unknown_a);
    printf("  - Registration fee    : %d\n", trn.registration_free);
    printf("  - Initial value       : %d\n", trn.assumed_initial_value);
    printf("  - ID                  : %d\n", trn.tournament_id);
    printf("  - PIC file            : %s\n", trn.pic_file);
    
    if(output->count > 0) {
        ret = sd_tournament_save(&trn, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed to save TRN file to %s: %s\n",
                output->filename[0],
                sd_get_error(ret));
        }
    }

    sd_tournament_free(&trn);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}