/** @file main.c
  * @brief SOUNDS.DAT file editor / player
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <ao/ao.h>
#include <argtable2.h>
#include <shadowdive/shadowdive.h>

int main(int argc, char* argv[]) {
    char error[128];
    int retcode = 0;

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "SOUNDS.DAT file");
    struct arg_int *sid = arg_int0("s", "sid", "<int>", "SampleID to play");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,sid,end};
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
    sd_sound_file *sounds = sd_sounds_create();
    retcode = sd_sounds_load(sounds, file->filename[0]);
    if(retcode) {
        sd_get_error(error, retcode);
        printf("Error: %s\n", error);
        goto exit_1;
    }
    
    int id = sid->ival[0]-1;
    if(sid->count > 0) {
        printf("Attempting to play sample #%d.\n", id+1);
    
        // Make sure there is data at requested ID position
        if(sounds->sounds[id] == 0) {
            printf("Sample does not contain data.\n");
            goto exit_1;
        }
    
        // Initialize libao
        ao_initialize();
        
        // Some required vars ...
        int driver;
        ao_device* output;
        ao_sample_format format;
            
        // Set format
        format.bits = 8;
        format.rate = 8000;
        format.channels = 1;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = NULL;
        
        // Open live output dev
        driver = ao_default_driver_id();
        output = ao_open_live(driver, &format, NULL);
        if(output == NULL) {
            printf("Unable to open output device!\n");
            goto exit_2;
        }
        
        // Play
        printf("Playing ...");
        ao_play(output, sounds->sounds[id]->data, sounds->sounds[id]->len);
        printf(" done.\n");
       
        // All done, free resources
        ao_close(output);
exit_2:
        ao_shutdown();
    } else {
        printf("Valid sample ID's:\n");
        int k = 0;
        for(int i = 0; i < sounds->sound_count; i++) {
            if(sounds->sounds[i]) {
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
    sd_sounds_delete(sounds);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
