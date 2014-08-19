/** @file main.c
  * @brief SOUNDS.DAT file editor / player
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <SDL2/SDL.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

typedef struct _streamer {
    char *data;
    int pos;
    int size;
} Streamer;

void stream(void* userdata, Uint8* stream, int len) {
    Streamer *s = (Streamer*)userdata;
    SDL_memset(stream, 128, len);
    int left = s->size - s->pos;
    len = (len > left) ? left : len;
    SDL_memcpy(stream, s->data + s->pos, len);
    s->pos += len;
    printf("Reading %d, size %d, left %d\n", len, s->size, (s->size-s->pos));
}

int main(int argc, char *argv[]) {
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    Streamer streamer;
    int retcode = 0;

    // Init SDL Audio
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "SOUNDS.DAT file");
    struct arg_int *sid = arg_int0("s", "sid", "<int>", "SampleID to play");
    struct arg_int *sampleprint = arg_int0("p", "print", "<int>", "Samples to print");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,sid,sampleprint,end};
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
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }
    
    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 SOUNDS.DAT file editor.\n");
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
    
    // Open sounds.dat
    sd_sound_file sf;
    sd_sounds_create(&sf);
    retcode = sd_sounds_load(&sf, file->filename[0]);
    if(retcode) {
        printf("Error %d: %s\n", retcode, sd_get_error(retcode));
        goto exit_1;
    }
    
    int id = sid->ival[0]-1;
    if(sid->count > 0 && sampleprint->count > 0) {
        int scount = sampleprint->ival[0];
        printf("Sample size = %d\n", sf.sounds[id]->len);
        printf("Attempting to print #%d first samples.\n", scount);
        for(int i = 0; i < scount; i++) {
            unsigned int s = sf.sounds[id]->data[i] & 0xFF;
            printf("%2x ", s);
        }
    } else if(sid->count > 0) {
        printf("Attempting to play sample #%d.\n", id+1);
    
        // Make sure there is data at requested ID position
        if(sf.sounds[id] == 0) {
            printf("Sample does not contain data.\n");
            goto exit_1;
        }
    
        // Streamer
        streamer.size = sf.sounds[id]->len;
        streamer.pos = 0;
        streamer.data = sf.sounds[id]->data;
    
        // Initialize required audio
        SDL_zero(want);
        want.freq = 8000;
        want.format = AUDIO_U8;
        want.channels = 1;
        want.samples = 4096;
        want.callback = stream;
        want.userdata = &streamer;
            
        // Open device, play file
        dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
        if(dev == 0) {
            printf("Failed to open audio dev: %s\n", SDL_GetError());
            goto exit_0;
        } else {
            if(have.format != want.format) {
                printf("Could not get correct playback format.\n");
            } else {
                printf("Starting playback ...\n");
                SDL_PauseAudioDevice(dev, 0);
                while(streamer.pos < streamer.size) {
                    SDL_Delay(100);
                }
                printf("All done.\n");
            }
            SDL_CloseAudioDevice(dev);
        }
    } else {
        printf("Valid sample ID's:\n");
        int k = 0;
        for(int i = 0; i < sf.sound_count; i++) {
            if(sf.sounds[i] != NULL) {
                printf("%d", i+1);
                if((k+1)%6==0) {
                    printf("\n");
                } else {
                    printf("\t");
                }
                k++;
            }
        }
        printf("\n");
    }
    

exit_1:
    sd_sounds_free(&sf);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
