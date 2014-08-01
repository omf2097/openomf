/** @file main.c
  * @brief .PSM music file player tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <string.h>

#ifdef USE_MODPLUG
    #include <libmodplug/modplug.h>
#else
    #include <dumb/dumb.h>
#endif

#define PROGRESSBAR_LENGTH 25
#define CHANNELS 2
#define FREQUENCY 44100
#define RESAMPLER 3
#define DELTA (65536.0f / FREQUENCY)

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

void format_ms(int ticks) {
    int total_seconds = ticks / 1000;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    
    if(total_seconds > 3600) {
        hours = total_seconds / 3600;
        total_seconds = total_seconds % 3600;
    }
    if(total_seconds > 60) {
        minutes = total_seconds / 60;
        total_seconds = total_seconds % 60;
    }
    seconds = total_seconds;
    
    if(hours > 0)
        printf("%02d:%02d:%02d", hours, minutes, seconds);
    else {
        printf("%02d:%02d", minutes, seconds);
    } 
}

void show_progress(int width, float progress, int ticks) {
    int d = progress * width;
 
    printf("%3d%% [", (int)(progress*100));
    for(int x = 0; x < d; x++) {
        printf("=");
    }
    for(int x = 0; x < (width-d); x++) {
        printf(" ");
    }
    printf("] ");
    format_ms(ticks);
    
    printf("\r");
}

const char* get_file_ext(const char* filename) {
    return strrchr(filename, '.') + 1;
}

const char* resample_filter_name(int id) {
    switch(id) {
        case DUMB_RQ_ALIASING: return "Aliasing";
        case DUMB_RQ_LINEAR: return "Linear";
        case DUMB_RQ_CUBIC: return "Cubic";
        case DUMB_RQ_FIR: return "FIR";
    }
    return "Unknown";
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
    
    // Load correct file type
    const char *ext = get_file_ext(file->filename[0]);
    if(!ext) {
        printf("Could not find file extension.");
        goto exit_0;
    }
    
    // Load up file
    if(strcasecmp(ext, "psm") == 0) {
        src_data = dumb_load_psm(file->filename[0], 0);
    } else if(strcasecmp(ext, "mod") == 0) {
        src_data = dumb_load_mod(file->filename[0], 0);
    } else if(strcasecmp(ext, "xm") == 0) {
        src_data = dumb_load_xm(file->filename[0]);
    } else if(strcasecmp(ext, "s3m") == 0) {
        src_data = dumb_load_s3m(file->filename[0]);
    } else if(strcasecmp(ext, "it") == 0) {
        src_data = dumb_load_it(file->filename[0]);
    } else {
        printf("Unknown module format.");
        goto exit_0;
    }
    
    // Initialize dumb renderer
    if(!src_data) {
        printf("Unable to load file!\n");
        goto exit_0;
    }
    src_renderer = duh_start_sigrenderer(src_data, 0, CHANNELS, 0);
    dumb_resampling_quality = RESAMPLER;
    printf("File '%s' loaded succesfully.\n", file->filename[0]);
    
    // Streamer
    streamer.renderer = src_renderer;
    streamer.size = duh_get_length(src_data);
    streamer.pos = 0;
    streamer.ended = 0;
    
    // SDL2
    SDL_zero(want);
    want.freq = FREQUENCY;
    want.format = AUDIO_S16;
    want.channels = CHANNELS;
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
        
        // Some information
        printf("Resample filter: %s\n", resample_filter_name(dumb_resampling_quality));
        printf("Frequency:       %d\n", FREQUENCY);
        printf("Channels:        %d\n", CHANNELS);
        printf("Extension:       %s\n", get_file_ext(file->filename[0]));
        
        // Play file
        printf("Starting playback.\n");
        SDL_PauseAudioDevice(dev, 0);
        show_progress(PROGRESSBAR_LENGTH, 0.0f, 0);
        int time_start = SDL_GetTicks();
        while(!streamer.ended) {
            SDL_Delay(250);
            float seek = ((float)streamer.pos) / ((float)streamer.size);
            int ms_played = SDL_GetTicks() - time_start;
            show_progress(PROGRESSBAR_LENGTH, seek, ms_played);
        }
        printf("\nAll done.\n");
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
