/** @file main.c
  * @brief .AF file editor tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .AF file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .AF file");
    struct arg_file *palette = arg_file0("p", "palette", "<file>", "BK file for palette");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,palette,end};
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
        printf("Command line One Must Fall 2097 .AF file editor.\n");
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
    
    // Init SDL
    SDL_Init(SDL_INIT_VIDEO);
    
    // Load file
    sd_af_file *af = sd_af_create();
    int ret = sd_af_load(af, file->filename[0]);
    if(ret) {
        printf("Unable to load AF file! Make sure the file exists and is a valid AF file.\n");
        goto exit_1;
    }
    
    // TODO: Handle everything here
    
    
    // Write output file
    if(output->count > 0) {
        sd_af_save(af, output->filename[0]);
    }
    
    // Quit
exit_1:
    sd_af_delete(af);
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}