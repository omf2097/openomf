/** @file main.c
 * @brief SETUP.CFG file editor tool
 * @license MIT
 */

#include "../shared/pilot.h"
#include "formats/error.h"
#include "formats/setup.h"
#include "utils/c_array_util.h"
#if defined(ARGTABLE2_FOUND)
#include <argtable2.h>
#elif defined(ARGTABLE3_FOUND)
#include <argtable3.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *shadows[] = {
    "None",
    "Low",
    "Medium",
    "High",
};

static const char *onoff[] = {"Off", "On"};

static const char *difficulty_list[] = {"Punching bag", "Rookia", "Veteran", "World Class",
                                        "Champion",     "Deadly", "Ultimate"};

static const char *match_type[] = {"One match", "2 out of 3", "3 out of 5", "4 out of 7"};

static const char *knockdown_text[] = {"None", "Kicks", "Punches", "Kicks & Punches"};

void print_setup_root_info(sd_setup_file *setup) {
    if(setup == NULL) {
        return;
    }

    printf(" - Speed:         %d\n", setup->game_speed);
    printf(" - Unknown A:\n");
    print_bytes(setup->unknown_a, sizeof(setup->unknown_a), 32, 3);
    printf("\n");
    printf(" - Unknown B:     %d\n", setup->unknown_b);
    printf(" - Unknown C:     %d\n", setup->unknown_c);
    printf(" - Unknown D:     %d\n", setup->unknown_d);
    printf(" - Unknown E:\n");
    print_bytes(setup->unknown_e, sizeof(setup->unknown_e), 32, 3);
    printf("\n");
    printf(" - Difficulty:    %s\n", difficulty_list[setup->difficulty]);
    printf(" - Unknown G:     %d\n", setup->unknown_g);
    printf(" - Throw range:   %d\n", setup->throw_range);
    printf(" - Hit pause:     %d\n", setup->hit_pause);
    printf(" - Block Damage:  %d\n", setup->block_damage);
    printf(" - Vitality:      %d\n", setup->vitality);
    printf(" - Jump height:   %d\n", setup->jump_height);

    printf(" - Flags 0:\n");
    printf("    * unk1:       %d\n", setup->general_flags_0.unk);
    printf("    * rehit_mode: %d\n", setup->general_flags_0.rehit_mode);
    printf("    * def_throws: %d\n", setup->general_flags_0.def_throws);
    printf("    * unk2:       %d\n", setup->general_flags_0.unk2);
    printf("    * Power 1:    %d\n", setup->general_flags_0.power_1);
    printf("    * unk3:       %d\n", setup->general_flags_0.unk3);
    printf("    * Power 2:    %d\n", setup->general_flags_0.power_2);
    printf("    * unk4:       %d\n", setup->general_flags_0.unk4);

    printf(" - Flags 1:\n");
    printf("    * knockdown:  %s\n", knockdown_text[setup->general_flags_1.knockdown]);
    printf("    * shadows:    %s\n", shadows[setup->general_flags_1.shadows]);
    printf("    * hazards:    %s\n", onoff[setup->general_flags_1.hazards]);
    printf("    * hyper mode: %s\n", onoff[setup->general_flags_1.hyper_mode]);
    printf("    * screen shk: %s\n", onoff[setup->general_flags_1.screen_shakes]);
    printf("    * animations: %s\n", onoff[setup->general_flags_1.animations]);

    printf(" - Flags 2:\n");
    printf("    * pal anim:   %s\n", onoff[setup->general_flags_2.palette_animation]);
    printf("    * unk2:       %d\n", setup->general_flags_2.unk2);
    printf("    * snow chk:   %s\n", onoff[setup->general_flags_2.snow_checking]);
    printf("    * unk3:       %d\n", setup->general_flags_2.unk3);

    printf(" - Flags 3:\n");
    printf("    * stereo rev: %s\n", onoff[setup->general_flags_3.stereo_reversed]);
    printf("    * match cnt:  %s\n", match_type[setup->general_flags_3.match_count]);
    printf("    * unk:        %d\n", setup->general_flags_3.unk);

    printf(" - Sound volume:  %d\n", setup->sound_volume);
    printf(" - Music volume:  %d\n", setup->music_volume);
    printf(" - Unknown R:\n");
    print_bytes(setup->unknown_r, sizeof(setup->unknown_r), 32, 3);
    printf("\n");
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, end};
    const char *progname = "setuptool";

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
    arg_freetable(argtable, N_ELEMENTS(argtable));
    return 0;
}
