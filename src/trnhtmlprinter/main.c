/** @file main.c
  * @brief .TRN file html printer
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <png.h>

const char *header = "<!DOCTYPE html>\
<html>\
<head>\
<title>TrnHtmlPrinter</title>\
<link href='https://fonts.googleapis.com/css?family=Open+Sans:400,700,800' rel='stylesheet' type='text/css'>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js'></script>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jqueryui/1.10.2/jquery-ui.min.js'></script>\
<script type='text/javascript'>\
$(function() {\
$(\"#pilots\").accordion({heightStyle: 'content'});\
$(\"#locales\").accordion({heightStyle: 'content'});\
});\
</script>\
<style type='text/css'>\
body,p,table,tr,th,td { font-family: 'Open Sans', sans-serif; }\
table { margin:0;padding:0; border-spacing:0; border-collapse:collapse; }\
td,th { padding-left: 5px; padding-right: 5px; padding-top: 3px; padding-bottom: 3px; text-align: left; font-size: 14px; }\
.ui-accordion-content td { border: 1px solid #dedede; }\
.ui-accordion-content th { background-color: #dedede; }\
h1 { font-size: 30px; font-weight: bold; }\
h2 { font-size: 24px; font-weight: bold; }\
h3 { font-size: 18px; font-weight: bold; }\
h4 { font-size: 14px; font-weight: bold; }\
.ui-accordion-header { padding-left: 15px; border: 1px solid #bbbbbb; }\
.ui-accordion-content {padding-bottom: 4px; padding-left: 15px; padding-right: 15pc; border: 1px dotted #bbbbbb;}\
.ui-accordion-content .iblock { display: inline-block; margin: 10px; padding: 10px; background-color: #eeeeee; }\
</style>\
</head>\
<body>";

const char *footer = "</body></html>";

int write_png(FILE *fp, char *data, int w, int h) {
    png_structp png_ptr;
    png_infop info_ptr;

    // Get row pointers
    char *rows[h];
    for(int i = 0; i < h; i++) {
        rows[i] = data + (i * w) * 4;
    }

    // Init
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);

    // Write header. RGB, 8bits per channel
    setjmp(png_jmpbuf(png_ptr));
    png_set_IHDR(png_ptr,
                 info_ptr,
                 w,
                 h,
                 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    // Write data
    setjmp(png_jmpbuf(png_ptr));
    png_write_image(png_ptr, (void*)rows);

    // End
    setjmp(png_jmpbuf(png_ptr));
    png_write_end(png_ptr, NULL);
    return 0;
}

// Main --------------------------------------------------------------

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .TRN file");
    struct arg_str *outdir = arg_str1("o", "outdir", "<str>", "Output directory");
    struct arg_str *name = arg_str1("n", "name", "<str>", "Output name");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,outdir,name,end};
    const char* progname = "trnhtmlprinter";

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
        arg_print_glossary(stdout, argtable, "%-30s %s\n");
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        printf("%s v0.1\n", progname);
        printf("Command line One Must Fall 2097 .TRN html printer.\n");
        printf("Source code is available at https://github.com/omf2097 under MIT license.\n");
        printf("(C) 2015 Tuomas Virtanen\n");
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Load file
    sd_tournament_file trn;
    sd_tournament_create(&trn);
    int ret = sd_tournament_load(&trn, file->filename[0]);
    if(ret) {
        printf("Unable to load TRN file! Make sure the file exists and is a valid BK file.\n");
        goto exit_1;
    }

    // Some vars
    char namebuf[256];

    // Open output file
    FILE *f;
    sprintf(namebuf, "%s/%s.html", outdir->sval[0], name->sval[0]);
    f = fopen(namebuf, "w");
    if(f == NULL) {
        printf("Error while opening file!");
        goto exit_1;
    }

    // Print header to file
    fprintf(f, "%s", header);
    fprintf(f, "<h1>%s</h1>", name->sval[0]);

    // Root
    fprintf(f, "<h2>Details</h2><table><tr><th>Key</th><th>Value</th></tr>");
    fprintf(f, "<tr><td>BK name</td><td>%s</td></tr>", trn.bk_name);
    fprintf(f, "<tr><td>Enemy count</td><td>%d</td></tr>", trn.enemy_count);
    fprintf(f, "<tr><td>Winnings multiplier</td><td>%f</td></tr>", trn.winnings_multiplier);
    fprintf(f, "<tr><td>Registration fee</td><td>%d</td></tr>", trn.registration_free);
    fprintf(f, "<tr><td>Assumed initial value</td><td>%d</td></tr>", trn.assumed_initial_value);
    fprintf(f, "<tr><td>Unknown A:</td><td>%d</td></tr>", trn.unknown_a);
    fprintf(f, "<tr><td>Tournament ID</td><td>%d</td></tr>", trn.tournament_id);
    fprintf(f, "<tr><td>PIC File</td><td>%s</td></tr>", trn.pic_file);
    fprintf(f, "</table>");

    // Palette
    sd_palette *pal = &trn.pal;
    fprintf(f, "<h2>Palette</h2>");
    fprintf(f, "<table>");
    for(int y = 0; y < 16; y++) {
        fprintf(f, "<tr>");
        for(int x = 0; x < 16; x++) {
            fprintf(f, "<td style=\"background-color: rgb(%d,%d,%d); text-align: middle; width: 30px; height: 30px; color: white;\">%d</td>",
                pal->data[y*16+x][0],
                pal->data[y*16+x][1],
                pal->data[y*16+x][2],
                y*16 + x);
        }
        fprintf(f, "</tr>");
    }
    fprintf(f, "</table>");

    // Print all pilot information
    fprintf(f, "<h2>Pilots</h2><div id=\"pilots\">");
    for(int m = 0; m < trn.enemy_count; m++) {
        sd_pilot *enemy = trn.enemies[m];
        fprintf(f, "<h3>Pilot %d %s</h3><div class=\"pilot\">", m, enemy->name);

        // Pilot information
        fprintf(f, "<h4>Information</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><th>Key</th><th>Value</th></tr>");
        fprintf(f, "<tr><td>Unknown A</td><td>%d</td></tr>", enemy->unknown_a);
        fprintf(f, "<tr><td>Wins</td><td>%d</td></tr>", enemy->wins);
        fprintf(f, "<tr><td>Losses</td><td>%d</td></tr>", enemy->losses);
        fprintf(f, "<tr><td>Rank</td><td>%d</td></tr>", enemy->rank);
        fprintf(f, "<tr><td>Har ID</td><td>%d</td></tr>", enemy->har_id);
        fprintf(f, "<tr><td>Tournament name</td><td>%s</td></tr>", enemy->trn_name);
        fprintf(f, "<tr><td>Tournament description</td><td>%s</td></tr>", enemy->trn_desc);
        fprintf(f, "<tr><td>Tournament image</td><td>%s</td></tr>", enemy->trn_image);
        fprintf(f, "</table>");

        // Print pilot palette
        fprintf(f, "<h4>Palette</h4>");
        fprintf(f, "<table>");
        for(int y = 0; y < 16; y++) {
            fprintf(f, "<tr>");
            for(int x = 0; x < 16; x++) {
                fprintf(f, "<td style=\"background-color: rgb(%d,%d,%d); text-align: middle; width: 30px; height: 30px; color: white;\">%d</td>",
                    enemy->palette.data[y*16+x][0],
                    enemy->palette.data[y*16+x][1],
                    enemy->palette.data[y*16+x][2],
                    y*16 + x);
            }
            fprintf(f, "</tr>");
        }
        fprintf(f, "</table>");

        // End .pilot infodiv
        fprintf(f, "</div>");
    }
    fprintf(f, "</div>");

    // Print all locale information
    fprintf(f, "<h2>Locales</h2><div id=\"locales\">");
    for(int m = 0; m < MAX_TRN_LOCALES; m++) {
        sd_tournament_locale *locale = trn.locales[m];
        if(locale == NULL)
            continue;

        // Title & index
        fprintf(f, "<h3>Locale %d %s</h3><div class=\"locale\">", m, locale->title);

        // Description text
        fprintf(f, "<h4>Description</h4>");
        fprintf(f, "<i>%s</i>", locale->description);

        // End texts
        fprintf(f, "<h4>End texts</h4>");
        fprintf(f, "<table>");
        fprintf(f, "<tr><th>HAR</th><th>Page</th><th>Text</th></tr>");
        for(int k = 0; k < 11; k++) {
            for(int g = 0; g < 10; g++) {
                if(locale->end_texts[k][g] != NULL && locale->end_texts[k][g][0] != 0) {
                    fprintf(f,
                        "<tr><td>%d</td><td>%d</td><td>%s</td></tr>",
                        k, g, locale->end_texts[k][g]);
                }
            }
        }
        fprintf(f, "</table>");

        fprintf(f, "</div>");
    }
    fprintf(f, "</div>");

    // Print footer to file
    fprintf(f, "%s", footer);

    // Quit
    fclose(f);
exit_1:
    sd_tournament_free(&trn);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
