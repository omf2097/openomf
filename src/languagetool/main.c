/** @file main.c
  * @brief Language file parser tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char *argv[]) {
   // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "language file");
    struct arg_int *str = arg_int1("s", "string", "<value>", "print language string # (-1 for all).");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,str,end};
    const char* progname = "languagetool";
    
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
        printf("Command line One Must Fall 2097 Language file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
    }
    
    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }
    
    // Get strings
    sd_language language;
    sd_language_create(&language);
    int ret = sd_language_load(&language, file->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("Language file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
        goto exit_0;
    }
    
    // Print
    int id = str->ival[0];
    if(id < 0) {
        for(int i = 0; i < language.count; i++) {
            printf("Title: %s\n", language.strings[i].description);
            printf("Data: %s\n", language.strings[i].data);
        }
    } else if(id >= 0 && id < language.count) {
        printf("Title: %s\n", language.strings[id].description);
        printf("Data: %s\n", language.strings[id].data);
    } else {
        printf("String not found!\n");
    }
    
    sd_language_free(&language);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}