/** @file main.c
 * @brief SOUNDS.DAT file editor / player
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/error.h"
#include "formats/sounds.h"
#include <SDL.h>
#include <argtable2.h>

typedef struct _streamer {
    char *data;
    int pos;
    int size;
} Streamer;

void stream(void *userdata, Uint8 *stream, int len) {
    Streamer *s = (Streamer *)userdata;
    SDL_memset(stream, 128, len);
    int left = s->size - s->pos;
    len = (len > left) ? left : len;
    SDL_memcpy(stream, s->data + s->pos, len);
    s->pos += len;
    printf("Reading %d, size %d, left %d\n", len, s->size, (s->size - s->pos));
}

int main(int argc, char *argv[]) {
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID dev;
    Streamer streamer;
    int retcode = 0;

    // Init SDL Audio
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "SOUNDS.DAT file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output sounds file");
    struct arg_int *sid = arg_int0("s", "sound", "<int>", "Sound ID");
    struct arg_int *sampleprint =
        arg_int0(NULL, "print", "<int>", "Print first n bytes from selected sound");
    struct arg_lit *play = arg_lit0("p", "play", "Play selected sound");
    struct arg_file *export =
        arg_file0("e", "export", "<file>", "Export selected sound to AU file");
    struct arg_file *import =
        arg_file0("i", "import", "<file>", "Import selected sound from AU file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, output, sid, sampleprint, play, export, import, end};
    const char *progname = "soundtool";

    // Make sure everything got allocated
    if (arg_nullcheck(argtable) != 0) {
        printf("%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("\nArguments:\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit_0;
    }

    // Handle version
    if (vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 SOUNDS.DAT file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2013 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if (nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Open sounds.dat
    sd_sound_file sf;
    sd_sounds_create(&sf);
    retcode = sd_sounds_load(&sf, file->filename[0]);
    if (retcode) {
        printf("Error %d: %s\n", retcode, sd_get_error(retcode));
        goto exit_1;
    }

    if (sid->count > 0) {
        // Sound ID to handle
        int sound_id = sid->ival[0];
        const sd_sound *sound = sd_sounds_get(&sf, sound_id - 1);
        if (sound == NULL) {
            printf("Invalid sound ID");
            goto exit_1;
        }

        if (sampleprint->count > 0) {
            int count = (sampleprint->ival[0] > sound->len) ? sound->len : sampleprint->ival[0];
            printf("Sample size = %d\n", sound->len);
            printf("Unknown = %d\n", sound->unknown);
            printf("Attempting to print %d first bytes.\n", count);
            for (int i = 0; i < count; i++) {
                unsigned int s = sound->data[i] & 0xFF;
                printf("%2x ", s);
            }
        } else if (play->count > 0) {
            printf("Attempting to play sample #%d.\n", sound_id);

            // Make sure there is data at requested ID position
            if (sound->len <= 0) {
                printf("Sample does not contain data.\n");
                goto exit_1;
            }

            // Streamer
            streamer.size = sound->len;
            streamer.pos = 0;
            streamer.data = sound->data;

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
            if (dev == 0) {
                printf("Failed to open audio dev: %s\n", SDL_GetError());
                goto exit_0;
            } else {
                if (have.format != want.format) {
                    printf("Could not get correct playback format.\n");
                } else {
                    printf("Starting playback ...\n");
                    SDL_PauseAudioDevice(dev, 0);
                    while (streamer.pos < streamer.size) {
                        SDL_Delay(100);
                    }
                    printf("All done.\n");
                }
                SDL_CloseAudioDevice(dev);
            }
        } else if (import->count > 0) {
            if (sd_sound_from_au(&sf, sound_id, import->filename[0]) != SD_SUCCESS) {
                printf("Importing sample %d from file %s failed.\n", sound_id, import->filename[0]);
            } else {
                printf("Importing sample %d from file %s succeeded.\n", sound_id,
                       import->filename[0]);
            }
        } else if (export->count > 0) {
            if (sd_sound_to_au(&sf, sound_id, export->filename[0]) != SD_SUCCESS) {
                printf("Exporting sample %d to file %s failed.\n", sound_id, export->filename[0]);
            } else {
                printf("Exporting sample %d to file %s succeeded.\n", sound_id,
                       export->filename[0]);
            }
        } else {
            printf("Selected sample %d\n", sound_id);
            printf("Unknown = %d\n", sound->unknown);
            printf("Length = %d\n", sound->len);
        }
    } else {
        printf("Valid sample ID's:\n");
        int k = 0;
        for (int i = 0; i < SD_SOUNDS_MAX; i++) {
            const sd_sound *sound = sd_sounds_get(&sf, i);
            if (sound != NULL && sound->len > 2) {
                printf("%d", i + 1);
                if ((k + 1) % 6 == 0) {
                    printf("\n");
                } else {
                    printf("\t");
                }
                k++;
            }
        }
        printf("\n");
    }

    if (output->count > 0) {
        if (sd_sounds_save(&sf, output->filename[0]) != SD_SUCCESS) {
            printf("Saving soundfile to %s failed.\n", output->filename[0]);
        }
    }

exit_1:
    sd_sounds_free(&sf);
exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
