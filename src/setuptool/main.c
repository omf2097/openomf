/** @file main.c
  * @brief SETUP.CFG file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include "../shared/pilot.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void print_setup_root_info(sd_setup_file *setup) {
    if(setup == NULL) {
        return;
    }

    printf(" - Unknown A:\n");
    print_bytes(setup->unknown_a, sizeof(setup->unknown_a), 32, 3);
    printf("\n");
    printf(" - Unknown B:    %d\n", setup->unknown_b);
    printf(" - Unknown C:    %d\n", setup->unknown_c);
    printf(" - Unknown D:    %d\n", setup->unknown_d);
    printf(" - Unknown E:\n");
    print_bytes(setup->unknown_e, sizeof(setup->unknown_e), 32, 3);
    printf("\n");
    printf(" - Unknown F:    %d\n", setup->unknown_f);
    printf(" - Unknown G:    %d\n", setup->unknown_g);
    printf(" - Unknown H:    %d\n", setup->unknown_h);
    printf(" - Unknown I:    %d\n", setup->unknown_i);
    printf(" - Unknown J:    %d\n", setup->unknown_j);
    printf(" - Unknown K:    %d\n", setup->unknown_k);
    printf(" - Unknown L:    %d\n", setup->unknown_l);
    printf(" - Unknown M:    %d\n", setup->unknown_m);
    printf(" - Flags:\n");
    printf("    * unk1:      %d\n", setup->general_flags.unk);
    printf("    * shadows:   %d\n", setup->general_flags.shadows);
    printf("    * unk2:      %d\n", setup->general_flags.unk2);
    printf(" - Unknown N:    %d\n", setup->unknown_n);
    printf(" - Unknown O:    %d\n", setup->unknown_o);
    printf(" - Unknown P:    %d\n", setup->unknown_p);
    printf(" - Unknown Q:    %d\n", setup->unknown_q);
    printf(" - Unknown R:\n");
    print_bytes(setup->unknown_r, sizeof(setup->unknown_r), 32, 3);
    printf("\n");
}

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,end};
    const char* progname = "setuptool";

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
        printf("Command line One Must Fall 2097 SETUP.CFG file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2015 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Load file
    sd_setup_file setup;
    sd_setup_create(&setup);
    if(file->count > 0) {
        int ret = sd_setup_load(&setup, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load setup file! [%d] %s.\n", ret, sd_get_error(ret));
            goto exit_1;
        }
    }

    print_setup_root_info(&setup);

    // Quit
exit_1:
    sd_setup_free(&setup);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
