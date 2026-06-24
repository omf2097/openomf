/** @file main.c
 * @brief Animation string parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/script.h"
#include "formats/tag_list_helpers.h"
#include "utils/c_array_util.h"
#include <argtable3.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_str *astr = arg_str1(NULL, NULL, "<str>", "Animation string");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, astr, end};
    const char *progname = "stringparser";
    char tmp_str[512];

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 Animation string parser\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle help
    if(help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    const char *str = *astr->sval;

    if(strcmp("-", *astr->sval) == 0) {
        if(fgets(tmp_str, 512, stdin) == NULL) {
            printf("Unexpectedly failed to read line from stdin\n");
            goto exit_0;
        }
        // throttle the newline
        tmp_str[strlen(tmp_str) - 1] = '\0';
        str = tmp_str;
    }

    // Print some data
    printf("Parsing \"%s\".\n\n", str);

    int err_pos;
    script script;
    script_create(&script);
    const int ret = script_decode(&script, str, &err_pos);
    if(ret != SD_SUCCESS) {
        if(ret == SD_INVALID_TAG) {
            printf("Bad input string! Error at position %d.\n", err_pos);
        }
        if(ret == SD_ANIM_INVALID_STRING) {
            printf("Bad input string!\n");
        }
        goto exit_1;
    }

    for(int frame_id = 0; frame_id < script_get_frame_count(&script); frame_id++) {
        const script_frame *frame = script_get_frame(&script, frame_id);
        printf("%d. Frame %d: '%c%d'\n", frame_id, frame->sprite, (char)(frame->sprite + 65), frame->tick_len);
        for(unsigned tag_id = 0; tag_id < vector_size(&frame->tags); tag_id++) {
            script_frame_tag *tag = vector_get(&frame->tags, tag_id);
            const char *desc = script_get_frame_tag_description(tag);
            if(desc == NULL) {
                desc = "";
            }
            if(tag->has_param) {
                printf("   %-4s %-4d %s\n", script_get_frame_tag_name(tag), tag->value, desc);
            } else {
                printf("   %-4s      %s\n", script_get_frame_tag_name(tag), desc);
            }
        }
    }

exit_1:
    script_free(&script);
exit_0:
    arg_freetable(argtable, N_ELEMENTS(argtable));
    return 0;
}
