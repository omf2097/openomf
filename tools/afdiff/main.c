/** @file main.c
  * @brief .AF file diff  tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <stdint.h>
#include <string.h>

#include "formats/af.h"
#include "formats/error.h"

#define VNAME(n) a->n, b->n

int int_diff(const char* label, int a, int b) {
    if(a != b) {
        printf("%-30s: %-6d != %-6d\n", label, a, b);
        return 1;
    }
    return 0;
}

int null_diff(const char* label, void *a, void *b) {
    if((a == NULL || b == NULL) && a != b) {
        printf("%-30s: %-6s != %-6s\n", label, (a ? "Exists" : "NULL"), (b ? "Exists" : "NULL"));
        return 1;
    }
    return 0;
}

int float_diff(const char* label, float a, float b) {
    if(a != b) {
        printf("%-30s: %-6f != %-6f\n", label, a, b);
        return 1;
    }
    return 0;
}

int str_diff(const char* label, const char *a, const char *b) {
    if(strcmp(a, b) != 0) {
        printf("%-30s: A      != B\n", label);
        return 1;
    }
    return 0;
}

int base_info_diff(sd_af_file *a, sd_af_file *b) {
    int d_found = 0;
    printf("Header:\n");
    d_found |= int_diff(" * File ID", VNAME(file_id));
    d_found |= int_diff(" * Exec window", VNAME(exec_window));
    d_found |= float_diff(" * Endurance", VNAME(endurance));
    d_found |= int_diff(" * Unknown B", VNAME(unknown_b));
    d_found |= int_diff(" * Health", VNAME(health));
    d_found |= float_diff(" * Fwd speed", VNAME(forward_speed));
    d_found |= float_diff(" * Rev speed", VNAME(reverse_speed));
    d_found |= float_diff(" * Jump speed", VNAME(jump_speed));
    d_found |= float_diff(" * Fall speed", VNAME(fall_speed));
    d_found |= int_diff(" * Unknown C", VNAME(unknown_c));
    d_found |= int_diff(" * Unknown D", VNAME(unknown_d));
    if(!d_found) {
        printf(" * No differences in header information.\n");
    }
    return d_found;
}

int anim_header_diff(sd_animation *a, sd_animation *b) {
    int d_found = 0;
    printf("   * Common animation header:\n");
    d_found |= int_diff("     + Start X", VNAME(start_x));
    d_found |= int_diff("     + Start Y", VNAME(start_y));
    d_found |= int_diff("     + Sprite count:", VNAME(sprite_count));
    d_found |= int_diff("     + Extra string count", VNAME(extra_string_count));
    d_found |= str_diff("     + Animation string", VNAME(anim_string));
    return d_found;
}

int anim_extrastrings_diff(sd_animation *a, sd_animation *b) {
    int d_found = 0;
    char k[20];
    if(a->extra_string_count == b->extra_string_count) {
        printf("   * Extra strings:\n");
        for(int i = 0; i < a->extra_string_count; i++) {
            sprintf(k, "     + %d", i);
            d_found |= str_diff(k, a->extra_strings[i], b->extra_strings[i]);
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
    for(int m = 0; m < 70; m++) {
        sprintf(f, " * %-2d", m);
        d_found |= null_diff(f, a->moves[m], b->moves[m]);
        if(a->moves[m] && b->moves[m]) {
            sd_move *am = a->moves[m];
            sd_move *bm = b->moves[m];
            sd_animation *aa = am->animation;
            sd_animation *ba = bm->animation;
            printf(" * %d:\n", m);
            d_found |= anim_common_diff(aa, ba);

            (void)(a->moves[m]);
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
        d_found |= int_diff(v, a->soundtable[k], b->soundtable[k]);
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

int main(int argc, char* argv[]) {
    sd_af_file af_a, af_b;
    int ret;

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *afile = arg_file1("a", "afile", "<file>", "First .AF file");
    struct arg_file *bfile = arg_file1("b", "bfile", "<file>", "Second .AF file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,afile,bfile,end};
    const char* progname = "aftool";
    
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
    
    // Load A file
    sd_af_create(&af_a);
    ret = sd_af_load(&af_a, afile->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("Unable to load AF file! [%d] %s.\n", ret, sd_get_error(ret));
        goto exit_1;
    }

    // Load B file
    sd_af_create(&af_b);
    ret = sd_af_load(&af_b, bfile->filename[0]);
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
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
