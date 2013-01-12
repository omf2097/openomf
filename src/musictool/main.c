/** @file main.c
  * @brief .PSM music file player tool
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <dumb/dumb.h>
#include <ao/ao.h>
#include <stdint.h>
#include <string.h>

#define DELTA (65536.0f / 44100)

int main(int argc, char *argv[]) {
    dumb_register_stdfiles();

    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_lit *loop = arg_lit0("l", "loop", "Loop playback");
    struct arg_file *file = arg_file1("f", "file", "<file>", "File to play");
    struct arg_file *export = arg_file0("e", "export", "<file>", "WAV file to export to");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,loop,file,export,end};
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
    
    // Some libdumb related stuff
    DUH *data;
    DUH_SIGRENDERER *renderer;
    
    // Load PSM file
    data = dumb_load_psm(file->filename[0], 0);
    if(!data) {
        printf("Unable to load file!\n");
        goto exit_0;
    }
    renderer = duh_start_sigrenderer(data, 0, 2, 0);
    printf("File loaded!\n");
    
    // Initialize libao
    ao_initialize();
    
    // Some required vars ...
    int driver;
    ao_device* output;
    ao_sample_format format;
        
    // Set format
    memset(&format, 0, sizeof(format));
    format.bits = 16;
    format.rate = 44100;
    format.channels = 2;
    format.byte_format = AO_FMT_LITTLE;
    format.matrix = NULL;
    
    // Export vs. play
    if(export->count > 0) {
        printf("Attempting to export to '%s'.\n", export->filename[0]);

        // Open output file
        driver = ao_driver_id("wav");
        output = ao_open_file(driver, export->filename[0], 0, &format, NULL);
        if(output == NULL) {
            printf("Could not open export file.");
            goto exit_1;
        }
        
        // Save to file
        printf("Converting ... ");
        long pos = 0;
        long len = duh_get_length(data);
        char buf[4096];
        while(pos < len) {
            duh_render(renderer, 16, 0, 1.0f, DELTA, 1024, buf);
            if(ao_play(output, buf, 4096) == 0) {
                printf(" Error while doing ao_play!\n");
                goto exit_2;
            }
            pos = duh_sigrenderer_get_position(renderer);
        }
        printf(" done.\n");
    } else {
        printf("Attempting to play file ...\n");
    
        // Open live output dev
        driver = ao_default_driver_id();
        output = ao_open_live(driver, &format, NULL);
        if(output == NULL) {
            printf("Unable to open output device!\n");
            goto exit_1;
        }
    
        // Play
        printf("Playing ...");
        long pos = 0;
        long len = duh_get_length(data);
        char buf[4096];
        while(pos < len || loop->count > 0) {
            duh_render(renderer, 16, 0, 1.0f, DELTA, 1024, buf);
            ao_play(output, buf, 4096);
            pos = duh_sigrenderer_get_position(renderer);
        }
        printf(" done.\n");
    }
    
    printf("Finished!\n");
exit_2:
    ao_close(output);
exit_1:
    ao_shutdown();
    duh_end_sigrenderer(renderer);
    unload_duh(data);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
