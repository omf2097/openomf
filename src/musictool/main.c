/** @file main.c
  * @brief .PSM music file player tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <dumb/dumb.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <string.h>

#define DELTA (65536.0f / 44100)

typedef struct _streamer {
    DUH_SIGRENDERER *renderer;
    int pos;
    int size;
    int ended;
} Streamer;

void stream(void* userdata, Uint8* stream, int len) {
    Streamer *s = (Streamer*)userdata;
    int before = s->pos;
    duh_render(s->renderer, 16, 0, 1.0f, DELTA, len/4, stream);
    s->pos = duh_sigrenderer_get_position(s->renderer);
    if(s->pos >= s->size || s->pos < before) {
        s->ended = 1;
    }
}

int main(int argc, char *argv[]) {
    DUH *src_data;
    DUH_SIGRENDERER *src_renderer;
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    Streamer streamer;

    // Init libs
    dumb_register_stdfiles();
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }

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
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 .PSM music file player.\n");
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
    
    // Load PSM file
    src_data = dumb_load_psm(file->filename[0], 0);
    if(!src_data) {
        printf("Unable to load file!\n");
        goto exit_0;
    }
    src_renderer = duh_start_sigrenderer(src_data, 0, 2, 0);
    printf("File loaded!\n");
    
    // Streamer
    streamer.renderer = src_renderer;
    streamer.size = duh_get_length(src_data);
    streamer.pos = 0;
    streamer.ended = 0;
    
    // SDL2
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 4096;
    want.callback = stream;
    want.userdata = &streamer;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if(dev == 0) {
        printf("Failed to open audio dev: %s\n", SDL_GetError());
        goto exit_0;
    } else {
        if(have.format != want.format) {
            printf("Could not get correct playback format.\n");
            goto exit_1;
        }
        
        printf("Starting playback ...\n");
        SDL_PauseAudioDevice(dev, 0);
        while(!streamer.ended) {
            SDL_Delay(5000);
            float seek = ((float)streamer.pos) / ((float)streamer.size) * 100.0f;
            printf("%d%% played.\n", (int)seek);
        }
        printf("All done.\n");
    }

exit_1:
    SDL_CloseAudioDevice(dev);
    duh_end_sigrenderer(src_renderer);
    unload_duh(src_data);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    SDL_Quit();
    return 0;
}
