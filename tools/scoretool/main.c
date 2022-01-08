/** @file main.c
 * @brief Score file parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/score.h"
#include <argtable2.h>

const char *page_name[] = {"ONE ROUND", "BEST 2 OF 3", "BEST 3 OF 5", "BEST 4 OF 7"};

const char *pilot_name[] = {"CRYSTAL", "STEFFAN", "MILANO",   "CHRISTIAN", "SHIRRO",   "JEAN-PAUL",
                            "IBRAHIM", "ANGEL",   "COSSETTE", "RAVEN",     "KREISSACK"};

const char *har_name[] = {"JAGUAR",   "SHADOW", "THORN",    "PYROS",   "ELECTRA", "KATANA",
                          "SHREDDER", "FLAIL",  "GARGOYLE", "CHRONOS", "NOVA"};

void print_page(sd_score *score, int page_id) {
    printf("PAGE %d: %s\n", page_id, page_name[page_id]);
    printf("%-18s%-9s%-9s%11s\n", "Player name", "HAR", "Pilot", "Score");
    for (int i = 0; i < SD_SCORE_ENTRIES; i++) {
        const sd_score_entry *entry = sd_score_get(score, page_id, i);
        if (entry->score > 0) {
            printf("%-18s%-9s%-9s%11d\n", entry->name, har_name[entry->har_id],
                   pilot_name[entry->pilot_id], entry->score);
        }
    }
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Score file");
    struct arg_int *page = arg_int0("p", "page", "<int>", "Page ID");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, page, output, end};
    const char *progname = "scoretool";

    // Make sure everything got allocated
    if (arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if (vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Score file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Get score information
    sd_score score;
    sd_score_create(&score);
    int ret = sd_score_load(&score, file->filename[0]);
    if (ret != SD_SUCCESS) {
        printf("Score file %s could not be loaded: %s\n", file->filename[0], sd_get_error(ret));
        goto exit_0;
    }

    // See if we want to print a single page or all pages
    if (page->count > 0) {
        int page_id = page->ival[0];
        if (page_id < 0 || page_id >= SD_SCORE_PAGES) {
            printf("Page must be between 0 and 3.\n");
            goto exit_1;
        }

        // Print only this page
        print_page(&score, page_id);
    } else {
        for (int i = 0; i < SD_SCORE_PAGES; i++) {
            print_page(&score, i);
            printf("\n");
        }
    }

    // Save if necessary
    if (output->count > 0) {
        ret = sd_score_save(&score, output->filename[0]);
        if (ret != SD_SUCCESS) {
            printf("Failed to save scores file to %s: %s\n", output->filename[0],
                   sd_get_error(ret));
        }
    }

exit_1:
    sd_score_free(&score);
exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
