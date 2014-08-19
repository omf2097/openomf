/** @file main.c
  * @brief .REC file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .REC file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,end};
    const char* progname = "rectool";
    
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
        printf("Command line One Must Fall 2097 .REC file editor.\n");
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
    
    // Load file
    sd_rec_file rec;
    sd_rec_create(&rec);
    if(file->count > 0) {
        int ret = sd_rec_load(&rec, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load REC file! [%d] %s.\n", ret, sd_get_error(ret));
            goto exit_1;
        }
    }
    
    // Write output file
    if(output->count > 0) {
        sd_rec_save(&rec, output->filename[0]);
    }
    
    // Quit
exit_1:
    sd_rec_free(&rec);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
