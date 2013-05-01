/** @file main.c
  * @brief .BK file html printer
  * @author Tuomas Virtanen
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>

const char *header = "<!DOCTYPE html>\
<html>\
<head>\
<title>BkHtmlPrinter</title>\
<link href='https://fonts.googleapis.com/css?family=Open+Sans:400,700,800' rel='stylesheet' type='text/css'>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js'></script>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jqueryui/1.10.2/jquery-ui.min.js'></script>\
<script type='text/javascript'>\
$(function() {\
$(\"#animations\").accordion({});\
});\
</script>\
<style type='text/css'>\
body,p,table,tr,th,td { font-family: 'Open Sans', sans-serif; }\
table { margin:0;padding:0; border-spacing:0; border-collapse:collapse; }\
td,th { padding-left: 5px; padding-right: 5px; padding-top: 3px; padding-bottom: 3px; text-align: left; font-size: 14px; }\
th { background-color: #dedede; }\
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

// Main --------------------------------------------------------------

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .BK file");
    struct arg_file *output = arg_file1("o", "output", "<file>", "Output .html file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,end};
    const char* progname = "bkhtmlprinter";
    
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
        printf("Command line One Must Fall 2097 .BK html printer.\n");
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
    
    // Load file
    sd_bk_file *bk = sd_bk_create();
    int ret = sd_bk_load(bk, file->filename[0]);
    if(ret) {
        printf("Unable to load BK file! Make sure the file exists and is a valid BK file.\n");
        goto exit_1;
    }
    
    // Open output file
    FILE *f;
    f = fopen(output->filename[0], "w");
    if(f == NULL) {
        printf("Error while opening file!");
        goto exit_1;
    }
    
    // Print header to file
    fprintf(f, "%s", header);
    fprintf(f, "<h1>%s</h1>", file->filename[0]);
    
    // Root
    fprintf(f, "<h2>General information</h2><table><tr><th>Key</th><th>Value</th></tr>");
    fprintf(f, "<tr><td>File ID</td><td>%d</td></tr>", bk->file_id);
    fprintf(f, "</table>");
    
    // Palettes
    if(bk->num_palettes > 0) {
        fprintf(f, "<h2>Palettes</h2>");
        for(int i = 0; i < bk->num_palettes; i++) {
            sd_palette *pal = bk->palettes[i];
            fprintf(f, "<h3>Palette %d</h3>", i+1);
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
        }
    }
    
    // Animations
    fprintf(f, "<h2>Animations</h2><div id=\"animations\">");
    for(int m = 0; m < 50; m++) {
        if(bk->anims[m]) {
            sd_bk_anim *bka = bk->anims[m];
            sd_animation *ani = bka->animation;
            fprintf(f, "<h3>Animation %d</h3><div class=\"animation\">", m);
            fprintf(f, "<div class=\"iblock\"><h4>General information</h4>");
            fprintf(f, "<table><tr><th>Key</th><th>Value</th></tr>");
            fprintf(f, "<tr><td>Null</td><td>%d</td></tr>", bka->null);
            fprintf(f, "<tr><td>Chain if hit</td><td>%d</td></tr>", bka->chain_hit);
            fprintf(f, "<tr><td>Chain if not hit</td><td>%d</td></tr>", bka->chain_no_hit);
            fprintf(f, "<tr><td>Load on start</td><td>%d</td></tr>", bka->load_on_start);
            fprintf(f, "<tr><td>Probability</td><td>%d</td></tr>", bka->probability);
            fprintf(f, "<tr><td>Hazard damage</td><td>%d</td></tr>", bka->hazard_damage);
            fprintf(f, "<tr><td>Unknown</td><td>%s</td></tr>", bka->unknown_data);
            
            fprintf(f, "<tr><td>Start X</td><td>%d</td></tr>", ani->start_x);
            fprintf(f, "<tr><td>Start Y</td><td>%d</td></tr>", ani->start_y);
            fprintf(f, "<tr><td>Animation string</td><td>%s</td></tr>", ani->anim_string);
            fprintf(f, "<tr><td>Unknown B</td><td>%d</td></tr>", ani->unknown_b);
            fprintf(f, "</table></div>");
            
            // Extra strings
            if(ani->extra_string_count) {
                fprintf(f, "<div class=\"iblock\"><h4>Extra strings</h4>");
                fprintf(f, "<table><tr><th>#</th><th>String</th></tr>");
                for(int e = 0; e < ani->extra_string_count; e++) {
                    fprintf(f, "<tr><td>%d</td><td>%s</td></tr>", e, ani->extra_strings[e]);
                }
                fprintf(f, "</table></div>");
            }
            
            // Coords
            if(ani->col_coord_count > 0) {
                fprintf(f, "<div class=\"iblock\"><h4>Collision coordinates</h4>");
                fprintf(f, "<table><tr><th>X</th><th>Y</th><th>X-ext</th><th>Y-ext</th></tr>");
                for(int c = 0; c < ani->col_coord_count; c++) {
                    col_coord *coord = &ani->col_coord_table[c];
                    fprintf(f, "<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>", coord->x, coord->y, coord->x_ext, coord->y_ext);
                }
                fprintf(f, "</table></div>");
            }
            
            // Frames
            fprintf(f, "<div class=\"iblock\"><h4>Frames</h4>");
            fprintf(f, "<table><tr><th>#</th><th>X</th><th>Y</th><th>W</th><th>H</th><th>Index</th><th>Missing</th></tr>");
            for(int b = 0; b < ani->frame_count; b++) {
                sd_sprite *sprite = ani->sprites[b];
                fprintf(f, "<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>", 
                    b,
                    sprite->pos_x,
                    sprite->pos_y,
                    sprite->img->w,
                    sprite->img->h,
                    sprite->index,
                    sprite->missing);
            }
            fprintf(f, "</table>");
            
            fprintf(f, "</div></div>");
        }
    }
    fprintf(f, "</div>");
    
    // Sounds
    fprintf(f, "<h2>Sound table</h2><table><tr><th>Local ID</th><th>Sound ID</th></tr>");
    for(int i = 0; i < 30; i++) {
        fprintf(f, "<tr><td>%d</td><td>%d</td></tr>", i, (int)bk->soundtable[i]);
    }
    fprintf(f, "</table>");
    
    
    // Print footer to file
    fprintf(f, "%s", footer);
    
    // Quit
    fclose(f);
exit_1:
    sd_bk_delete(bk);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
