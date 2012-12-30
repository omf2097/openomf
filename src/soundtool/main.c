#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,end};
    const char* progname = "soundtool";
    
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
        arg_print_glossary(stdout, argtable, "\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("'%s' Command line SOUNDS.DAT file editor.\n", progname);
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
    
    // TODO: Handle everything here
    
    // Quit
exit_1:
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}