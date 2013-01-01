#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <argtable2.h>
#include <dumb/dumb.h>
#include <inttypes.h>

#define DELTA (65536.0f / 44100)

void psm_player(void *udata, uint8_t *stream, int len) {
    DUH_SIGRENDERER *renderer = (DUH_SIGRENDERER*)udata;
    duh_render(renderer, 16, 0, 1.0f, DELTA, len / 4, stream);
}

int main(int argc, char *argv[]) {
    dumb_register_stdfiles();

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "File to play");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,end};
    const char* progname = "musictool";
    
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
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("'%s' Command line .PSM music file player.\n", progname);
        goto exit_0;
    }
    
    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }
    
    // argtable2 will make sure we have the -f parameter set
    // So just init and try to play ...
    
    // Initialize everything
    SDL_Init(SDL_INIT_AUDIO);
    printf("Attempting to play '%s' ...\n", file->filename[0]);
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 16384)) {
        printf("Failed to initialize SDL2_mixer!\n");
        goto exit_1;
    }
    printf("Audio channel opened.\n");
    
    // Some libdumb related stuff
    DUH *data;
    DUH_SIGRENDERER *renderer;
    
    // Load PSM file
    data = dumb_load_psm(file->filename[0], 0);
    if(!data) {
        printf("Unable to load file!\n");
        goto exit_2;
    }
    printf("File loaded!\n");
    
    // Kick the renderer up
    renderer = duh_start_sigrenderer(data, 0, 2, 0);
    
    // Hook custom playback function
    Mix_HookMusic(psm_player, renderer);
    
    // Idle while playing
    long pos = 0;
    long len = duh_get_length(data);
    int pc = 0;
    int last_pc = -1;
    printf("Position: 0%%");
    while(pos < len) {
        // Prevent looping
        pos = duh_sigrenderer_get_position(renderer);
        
        // Show data to user
        pc = (((float)pos / (float)len) * 100);
        if(pc % 10 == 0 && pc != last_pc && pc != 0) {
            printf(" ... %d%%", pc);
            last_pc = pc;
        }
        SDL_Delay(10);
    }
    printf("\n");

    // Deinit everything
    duh_end_sigrenderer(renderer);
    unload_duh(data);
exit_2:
    Mix_CloseAudio();
exit_1:
    SDL_Quit();
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}