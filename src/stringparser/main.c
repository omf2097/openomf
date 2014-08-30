/** @file main.c
  * @brief Animation string parser tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_str *astr = arg_str1("s", "str", "<str>", "Animation string");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,astr,end};
    const char* progname = "omf_parse";
    int from_stdin = 0;
    
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

    if (strcmp("-", *astr->sval) == 0) {
        from_stdin = 1;
        char *tmp_str = malloc(512);
        fgets(tmp_str, 512, stdin);
        // throttle the newline
        tmp_str[strlen(tmp_str)-1] = '\0';
        str = tmp_str;
    }

    // Print some data
    printf("Parsing \"%s\".\n\n", str);

    int err_pos;
    sd_script script;
    sd_script_create(&script);
    int ret = sd_script_decode(&script, str, &err_pos);
    if(ret != SD_SUCCESS) {
        printf("Bad input string! Error at position %d.\n", err_pos);
        goto exit_1;
    }

    for(int frame_id = 0; frame_id < script.frame_count; frame_id++) {
        sd_script_frame *frame = &script.frames[frame_id];
        printf("%d. Frame %d: '%c%d'\n",
            frame_id,
            frame->sprite,
            (char)(frame->sprite+65),
            frame->tick_len);
        for(int tag_id = 0; tag_id < frame->tag_count; tag_id++) {
            sd_script_tag *tag = &frame->tags[tag_id];
            if(tag->desc == NULL) {
                tag->desc = "";
            }
            if(tag->has_param) {
                printf("   %-4s %-4d %s\n", tag->key, tag->value, tag->desc);
            } else {
                printf("   %-4s      %s\n", tag->key, tag->desc);
            }
        }
    }

exit_1:
    sd_script_free(&script);
exit_0:
    if (from_stdin) {
        free((char*)str);
    }
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
