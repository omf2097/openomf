/** @file main.c
  * @brief Altpals file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input altpals file");
    struct arg_int *pal = arg_int1("p", "palette", "<number>", "Select a palette");
    struct arg_file *export = arg_file0("e", "export", "<file>", "Export selected palette to GPL file");
    struct arg_file *import = arg_file0("i", "import", "<file>", "Import selected palette from GPL file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output altpals file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,pal,output,import,export,end};
    const char* progname = "altpaltool";
    
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
        printf("Command line One Must Fall 2097 Altpals file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Need import or export ...
    if(pal->count > 0 && import->count == 0 && export->count == 0) {
        printf("Define either --import or --export with --palette!\n");
        goto exit_0;
    }

    // Make sure output is set
    if(import->count > 0 && output->count <= 0) {
        printf("Define --output with --import.\n");
        goto exit_0;
    }

    // Load file
    sd_altpal_file alt;
    sd_altpal_create(&alt);
    int ret = sd_altpals_load(&alt, file->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("Unable to load altpals file %s: %s.\n",
            file->filename[0],
            sd_get_error(ret));
        goto exit_1;
    }

    // Check ID
    int pal_id = pal->ival[0];
    if(pal_id < 0 || pal_id > SD_ALTPALS_PALETTES) {
        printf("Palette index %d does not exist!\n", pal_id);
        goto exit_1;
    }

    // Check what to do
    if(export->count > 0) {
        ret = sd_palette_to_gimp_palette(&alt.palettes[pal_id], export->filename[0]);
        if(ret == SD_SUCCESS) {
            printf("Palette %d exported to file %s succesfully.\n",
                pal_id,
                export->filename[0]);
        } else {
            printf("Error while attempting to save palette %d to file %s: %s",
                pal_id,
                export->filename[0],
                sd_get_error(ret));
        }
    } else if(import->count > 0) {
        ret = sd_palette_from_gimp_palette(&alt.palettes[pal_id], import->filename[0]);
        if(ret == SD_SUCCESS) {
            printf("Palette %d imported from file %s succesfully.\n",
                pal_id,
                import->filename[0]);
        } else {
            printf("Error while attempting to load palette %d from file %s: %s",
                pal_id,
                import->filename[0],
                sd_get_error(ret));
        }
    }
    
    // Write output file
    if(output->count > 0) {
        ret = sd_altpals_save(&alt, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed saving altpals file to %s: %s",
                output->filename[0],
                sd_get_error(ret));
        }
    }
    
    // Quit
exit_1:
    sd_altpal_free(&alt);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
