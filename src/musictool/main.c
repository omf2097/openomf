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
    #define MODPLUG_STATIC
    #include <libmodplug/modplug.h>
#else
    #include <dumb.h>
#endif

#define PROGRESSBAR_LENGTH 25
#define CHANNELS 2
#define FREQUENCY 44100
#define DELTA (65536.0f / FREQUENCY)

typedef struct _streamer {
#ifdef USE_MODPLUG
    ModPlugFile *renderer;
#else
    DUH_SIGRENDERER *renderer;
#endif
    int pos;
    int size;
    int ended;
} Streamer;

void stream(void* userdata, Uint8* stream, int len) {
    Streamer *s = (Streamer*)userdata;
    int before = s->pos;

#ifdef USE_MODPLUG
    int got = ModPlug_Read(s->renderer, stream, len);
    if(got == 0) {
        s->ended = 1;
    }
#else
    duh_render(s->renderer, 16, 0, 1.0f, DELTA, len/4, stream);
    s->pos = duh_sigrenderer_get_position(s->renderer);
#endif

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

char* get_file_data(const char *filename, size_t *size) {
    FILE *handle;
    char *data;

    // open handle, return NULL on error
    handle = fopen(filename, "rb");
    if(handle == NULL) {
        return NULL;
    }

    // Find size
    fseek(handle, 0L, SEEK_END);
    (*size) = ftell(handle);
    rewind(handle);

    // Read all data
    data = (char*)malloc(*size);
    fread(data, *size, sizeof(char), handle);
    fclose(handle);

    // Return data
    return data;
}

int main(int argc, char *argv[]) {
    // PSM reader stuff
#ifdef USE_MODPLUG
    ModPlug_Settings settings;
    char *src_data = NULL;
    size_t src_size;
    ModPlugFile *src_renderer = NULL;
#else
    DUH *src_data = NULL;
    DUH_SIGRENDERER *src_renderer = NULL;
#endif

    // SDL playback stuff
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    Streamer streamer;

    // Init libs
#ifdef USE_MODPLUG
#else
    dumb_register_stdfiles();
#endif

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

#ifdef USE_MODPLUG
    src_data = get_file_data(file->filename[0], &src_size);
    if(src_data == NULL) {
        printf("Unable to load file!\n");
        goto exit_0;
    }

    ModPlug_GetSettings(&settings);
    settings.mResamplingMode = MODPLUG_RESAMPLE_FIR;
    settings.mChannels = CHANNELS;
    settings.mBits = 16;
    settings.mFrequency = FREQUENCY;
    ModPlug_SetSettings(&settings);

    src_renderer = ModPlug_Load(src_data, src_size);
    if(!src_renderer) {
        printf("Unable to load file!\n");
        goto exit_0;
    }

    // Streamer
    streamer.renderer = src_renderer;
    streamer.size = ModPlug_GetLength(src_renderer);
    streamer.pos = 0;
    streamer.ended = 0;

#else // USE_MODPLUG
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

    // Streamer
    streamer.renderer = src_renderer;
    streamer.size = duh_get_length(src_data);
    streamer.pos = 0;
    streamer.ended = 0;
#endif // USE_MODPLUG

    printf("File '%s' loaded succesfully.\n", file->filename[0]);

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
#ifdef USE_MODPLUG
            streamer.pos = ms_played;
#endif
            show_progress(PROGRESSBAR_LENGTH, seek, ms_played);
        }
        printf("\nAll done.\n");
    }

exit_1:
    SDL_CloseAudioDevice(dev);
#ifdef USE_MODPLUG
    if(src_renderer)
        ModPlug_Unload(src_renderer);
    free(src_data);
#else
    if(src_renderer)
        duh_end_sigrenderer(src_renderer);
    unload_duh(src_data);
#endif
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    SDL_Quit();
    return 0;
}
