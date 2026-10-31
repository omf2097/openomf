/** @file main.c
 * @brief .AF file diff  tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include <argtable3.h>
#include <stdint.h>
#include <string.h>

#include "formats/af.h"
#include "formats/error.h"
#include "utils/c_array_util.h"

#define VNAME(n) a->n, b->n

int int_diff(const char *label, int a, int b) {
    if(a != b) {
        printf("%-30s: %-6d != %-6d\n", label, a, b);
        return 1;
    }
    return 0;
}

int null_diff(const char *label, void *a, void *b) {
    if((a == NULL || b == NULL) && a != b) {
        printf("%-30s: %-6s != %-6s\n", label, (a ? "Exists" : "NULL"), (b ? "Exists" : "NULL"));
        return 1;
    }
    return 0;
}

int float_diff(const char *label, float a, float b) {
    if(a != b) {
        printf("%-30s: %-6f != %-6f\n", label, a, b);
        return 1;
    }
    return 0;
}

int str_diff(const char *label, const char *a, const char *b) {
    if(strcmp(a, b) != 0) {
        printf("%-30s: A      != B\n", label);
        return 1;
    }
    return 0;
}

int base_info_diff(sd_af_file *a, sd_af_file *b) {
    int d_found = 0;
    printf("Header:\n");
    d_found |= int_diff(" * Fighter ID", VNAME(fighter_id));
    d_found |= int_diff(" * Exec window", VNAME(exec_window));
    d_found |= float_diff(" * Endurance", VNAME(endurance));
    d_found |= int_diff(" * Jump frame limit", VNAME(upwards_jump_frame_limit));
    d_found |= int_diff(" * Health", VNAME(health));
    d_found |= float_diff(" * Fwd speed", VNAME(forward_speed));
    d_found |= float_diff(" * Rev speed", VNAME(reverse_speed));
    d_found |= float_diff(" * Jump speed", VNAME(jump_speed));
    d_found |= float_diff(" * Fall speed", VNAME(fall_speed));
    d_found |= int_diff(" * Version 1", VNAME(version_1));
    d_found |= int_diff(" * AI projectile Y threshold", VNAME(ai_projectile_y_threshold));
    if(!d_found) {
        printf(" * No differences in header information.\n");
    }
    return d_found;
}

int anim_header_diff(sd_animation *a, sd_animation *b) {
    int d_found = 0;
    printf("   * Common animation header:\n");
    d_found |= int_diff("     + Start X", a->start_pos.x, b->start_pos.x);
    d_found |= int_diff("     + Start Y", a->start_pos.y, b->start_pos.y);
    d_found |= int_diff("     + Sprite count:", sd_animation_get_sprite_count(a), sd_animation_get_sprite_count(b));
    d_found |= int_diff("     + Extra string count", vector_size(&a->extra_strings), vector_size(&b->extra_strings));
    d_found |= str_diff("     + Animation string", str_c(&a->anim_string), str_c(&b->anim_string));
    return d_found;
}

int anim_extrastrings_diff(sd_animation *a, sd_animation *b) {
    int d_found = 0;
    char k[20];
    if(vector_size(&a->extra_strings) == vector_size(&b->extra_strings)) {
        printf("   * Extra strings:\n");
        for(unsigned int i = 0; i < vector_size(&a->extra_strings); i++) {
            sprintf(k, "     + %u", i);
            d_found |= str_diff(k, str_c(vector_get(&a->extra_strings, i)), str_c(vector_get(&b->extra_strings, i)));
        }
    }
    return d_found;
}

int anim_common_diff(sd_animation *a, sd_animation *b) {
    int d_found = 0;
    d_found |= anim_header_diff(a, b);
    d_found |= anim_extrastrings_diff(a, b);
    return d_found;
}

int animations_diff(sd_af_file *a, sd_af_file *b) {
    int d_found = 0;
    printf("Animations:\n");
    char f[20];
    for(int m = 0; m < MAX_AF_MOVES; m++) {
        sprintf(f, " * %-2d", m);
        sd_move *am = sd_af_get_move(a, m);
        sd_move *bm = sd_af_get_move(b, m);
        d_found |= null_diff(f, am, bm);
        if(am && bm) {
            sd_animation *aa = am->animation;
            sd_animation *ba = bm->animation;
            printf(" * %d:\n", m);
            d_found |= anim_common_diff(aa, ba);
        }
    }
    if(!d_found) {
        printf(" * No differences in animations.\n");
    }
    return d_found;
}

int soundtable_diff(sd_af_file *a, sd_af_file *b) {
    int d_found = 0;
    printf("Sound table:\n");
    char v[10];
    for(int k = 0; k < 30; k++) {
        sprintf(v, " * %-2d", k);
        d_found |= int_diff(v, a->sound_table[k], b->sound_table[k]);
    }
    if(!d_found) {
        printf(" * No differences in sound table.\n");
    }
    return d_found;
}

int af_diff(sd_af_file *a, sd_af_file *b) {
    int d_found = 0;
    d_found |= base_info_diff(a, b);
    printf("\n");
    d_found |= animations_diff(a, b);
    printf("\n");
    d_found |= soundtable_diff(a, b);
    printf("\n");
    if(!d_found) {
        printf("No differences between AF files found.\n");
    }
    return d_found;
}

int main(int argc, char *argv[]) {
    sd_af_file af_a, af_b;
    int ret;

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *afile = arg_file1("a", "afile", "<file>", "First .AF file");
    struct arg_file *bfile = arg_file1("b", "bfile", "<file>", "Second .AF file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, afile, bfile, end};
    const char *progname = "afdiff";

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
        printf("Command line One Must Fall 2097 .AF diff tool.\n");
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

    path file_a, file_b;
    path_from_c(&file_a, afile->filename[0]);
    path_from_c(&file_b, bfile->filename[0]);

    // Load A file
    sd_af_create(&af_a);
    ret = sd_af_load(&af_a, &file_a);
    if(ret != SD_SUCCESS) {
        printf("Unable to load AF file! [%d] %s.\n", ret, sd_get_error(ret));
        goto exit_1;
    }

    // Load B file
    sd_af_create(&af_b);
    ret = sd_af_load(&af_b, &file_b);
    if(ret != SD_SUCCESS) {
        printf("Unable to load AF file! [%d] %s.\n", ret, sd_get_error(ret));
        goto exit_2;
    }

    // Do the diff!
    af_diff(&af_a, &af_b);

    // Quit
exit_2:
    sd_af_free(&af_b);
exit_1:
    sd_af_free(&af_a);
exit_0:
    arg_freetable(argtable, N_ELEMENTS(argtable));
    return 0;
}
