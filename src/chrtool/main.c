/** @file main.c
  * @brief CHR file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../shared/pilot.h"

void print_chr_info(sd_chr_file *chr) {
    print_pilot_info(&chr->pilot);

    printf("\n");
    printf("  - Some unknown thingy: \n");
    print_bytes(chr->unknown, 20, 10, 5);
    printf("\n");

    // Portrait data
    printf("\n");
    printf("Portrait:\n");
    printf("  - Size = (%d,%d)\n", 
        chr->photo->width, 
        chr->photo->height);
    printf("  - Position = (%d,%d)\n", 
        chr->photo->pos_x, 
        chr->photo->pos_y);
    printf("  - Length = %d\n", 
        chr->photo->len);
    printf("  - I/M = %u/%u\n",
        chr->photo->index,
        chr->photo->missing);

    // Enemy data
    printf("\nEnemies:\n");
    for(int i = 0; i < chr->pilot.enemies_inc_unranked; i++) {
        printf("Enemy %d:\n", i);
        print_pilot_player_info(&chr->enemies[i]->pilot);
        printf("  - Some unknown thingy: \n");
        print_bytes(chr->enemies[i]->unknown, 20, 10, 5);
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input altpals file");
    struct arg_file *export = arg_file0("e","export","<file>","Export Photo to a ppm file");
    struct arg_file *bkfile = arg_file0("b","bkfile","<file>","Palette BK file");
    struct arg_file *output = arg_file0("o","output","<file>","Output CHR file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,export,bkfile,end};
    const char* progname = "chrtool";
    
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
        printf("Command line One Must Fall 2097 CHR file editor.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2014 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Require bkfile for export
    if(export->count > 0 && bkfile->count == 0) {
        printf("--bkfile is required for --export!\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Load file
    sd_chr_file chr;
    sd_chr_create(&chr);
    int ret = sd_chr_load(&chr, file->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("Unable to load chr file %s: %s.\n",
            file->filename[0],
            sd_get_error(ret));
        goto exit_1;
    }

    // Load bkfile if necessary
    sd_bk_file bk;
    if(bkfile->count > 0) {
        sd_bk_create(&bk);
        int ret = sd_bk_load(&bk, bkfile->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load BK file %s: %s.\n",
                bkfile->filename[0],
                sd_get_error(ret));
            goto exit_2;
        }
    }

    // Check if we want to export. If not, just print info
    if(export->count > 0) {
        sd_rgba_image img;
        sd_sprite_rgba_decode(
            &img,
            chr.photo,
            bk.palettes[0],
            -1);
        ret = sd_rgba_image_to_ppm(&img, export->filename[0]);
        if(ret == SD_SUCCESS) {
            printf("Exported photo to file %s.\n", export->filename[0]);
        } else {
            printf("Failed to export photo to file %s: %s\n",
                export->filename[0],
                sd_get_error(ret));
        }
        sd_rgba_image_free(&img);
    } else {
        print_chr_info(&chr);
    }

    // Saving
    if(output->count > 0) {
        ret = sd_chr_save(&chr, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed saving CHR file to %s: %s",
                output->filename[0],
                sd_get_error(ret));
        }
    }

    // Quit
exit_2:
    if(bkfile->count > 0) {
        sd_bk_free(&bk);
    }
exit_1:
    sd_chr_free(&chr);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
