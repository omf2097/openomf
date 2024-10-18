/** @file main.c
 * @brief PIC file parser tool
 * @author Tuomas Virtanen
 * @license MIT
 */

#include "formats/bk.h"
#include "formats/error.h"
#include "formats/pic.h"
#if ARGTABLE2_FOUND
#include <argtable2.h>
#elif ARGTABLE3_FOUND
#include <argtable3.h>
#endif

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "PIC file");
    struct arg_int *entry = arg_int0("p", "photo", "<int>", "Select PIC photo");
    struct arg_file *export = arg_file0("e", "export", "<file>", "Export selected photo sprite to ppm file");
    struct arg_file *bkfile = arg_file0("b", "bk", "<file>", "BK file to load palette from");
    struct arg_file *output = arg_file0("o", "output", "<file>", "PIC output file");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, vers, file, output, export, bkfile, entry, end};
    const char *progname = "pictool";

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
        printf("Command line One Must Fall 2097 PIC file editor.\n");
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

    // Notify about needing the BK file
    if(export->count > 0 && bkfile->count <= 0) {
        printf("For exporting, you need to define a BK file for palette.\n");
        goto exit_0;
    }

    // Get strings
    sd_pic_file pic;
    sd_pic_create(&pic);
    int ret = sd_pic_load(&pic, file->filename[0]);
    if(ret != SD_SUCCESS) {
        printf("PIC file could not be loaded! Error [%d] %s\n", ret, sd_get_error(ret));
        goto exit_0;
    }

    // Load bk file if necessary
    sd_bk_file bk;
    if(bkfile->count > 0) {
        sd_bk_create(&bk);
        ret = sd_bk_load(&bk, bkfile->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Could not load BK file: %s\n", sd_get_error(ret));
            goto exit_1;
        }
    }

    // Print
    const sd_pic_photo *photo;
    if(entry->count > 0) {
        int entry_id = entry->ival[0];
        photo = sd_pic_get(&pic, entry_id);
        if(photo == NULL) {
            printf("Photo %d does not exist.\n", entry_id);
            goto exit_2;
        }

        if(export->count > 0) {
            sd_rgba_image img;
            sd_sprite_rgba_decode(&img, photo->sprite, bk.palettes[0]);
            ret = sd_rgba_image_to_ppm(&img, export->filename[0]);
            if(ret != SD_SUCCESS) {
                printf("Failed to write photo %d to %s: %s\n", entry_id, export->filename[0], sd_get_error(ret));
            } else {
                printf("Photo %d exported to %s.\n", entry_id, export->filename[0]);
            }
            sd_rgba_image_free(&img);
        } else {
            printf("Length = %d\n", photo->sprite->len);
            printf("Size = (%d,%d)\n", photo->sprite->width, photo->sprite->height);
            printf("Position = (%d,%d)\n", photo->sprite->pos_x, photo->sprite->pos_y);
            printf("Sex = %s (%d)\n", (photo->sex ? "FEMALE" : "MALE"), photo->sex);
            printf("Is Player = %d\n", photo->is_player);
        }
    } else {
        printf("ID       Sex  Player  Length    W    H    X    Y  Unk\n");
        for(int i = 0; i < pic.photo_count; i++) {
            photo = sd_pic_get(&pic, i);
            printf("%3d %8s %7d %7d %4d %4d %4d %4d %4d\n", i, (photo->sex ? "FEMALE" : "MALE"), photo->is_player,
                   photo->sprite->len, photo->sprite->width, photo->sprite->height, photo->sprite->pos_x,
                   photo->sprite->pos_y, photo->unk_flag);
        }
    }

    if(output->count > 0) {
        ret = sd_pic_save(&pic, output->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Failed to save PIC file to %s: %s.", output->filename[0], sd_get_error(ret));
        }
    }

exit_2:
    if(export->count > 0) {
        sd_bk_free(&bk);
    }
exit_1:
    sd_pic_free(&pic);
exit_0:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}
