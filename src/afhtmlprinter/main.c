/** @file main.c
  * @brief .AF file html printer
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
<title>BkHtmlPrinter</title>\
<link href='https://fonts.googleapis.com/css?family=Open+Sans:400,700,800' rel='stylesheet' type='text/css'>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/1.8.3/jquery.min.js'></script>\
<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jqueryui/1.10.2/jquery-ui.min.js'></script>\
<script type='text/javascript'>\
$(function() {\
$(\"#animations\").accordion({heightStyle: 'content'});\
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

static const char *anim_names_a[] = {
    "",
    "Jumping",
    "Getting up",
    "Stunned",
    "Crouching",
    "Standing block", // 5
    "Crouching block", 
    "Burning oil",
    "Blocking scrape",
    "Damage",
    "Walking", // 10
    "Idle", 
    "Scrap",
    "Bolt",
    "Screw"
};

static const char *anim_names_b[] = {
    "Victory",
    "Loss"
};

static const char *anim_names_c[] = {
    "Blast 1",
    "Blast 2",
    "Blast 3"
};

const char *footer = "</body></html>";

int write_png(FILE *fp, char *data, int w, int h) {
    png_structp png_ptr;
    png_infop info_ptr;
    
    // Get row pointers
    char *rows[h];
    for(int i = 0; i < h; i++) {
        rows[i] = data + (i * w)*4;
    }
    
    // Init
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    setjmp(png_jmpbuf(png_ptr));
    png_init_io(png_ptr, fp);
    
    // Write header. RGB, 8bits per channel
    setjmp(png_jmpbuf(png_ptr));
    png_set_IHDR(png_ptr, info_ptr, w, h,
                 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
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
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .AF file");
    struct arg_file *palfile = arg_file1("p", "palfile", "<file>", "BK file for loading palette");
    struct arg_str *outdir = arg_str1("o", "outdir", "<str>", "Output directory");
    struct arg_str *name = arg_str1("n", "name", "<str>", "Output name");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,palfile,outdir,name,end};
    const char* progname = "afhtmlprinter";
    
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
        printf("Command line One Must Fall 2097 .AF html printer.\n");
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
    
    int ret;
    
    // Load palettes
    sd_bk_file bk;
    sd_bk_create(&bk);
    ret = sd_bk_load(&bk, palfile->filename[0]);
    if(ret) {
        printf("Unable to load %s file!", palfile->filename[0]);
        goto exit_1;
    }
    
    // Load file
    sd_af_file af;
    sd_af_create(&af);
    ret = sd_af_load(&af, file->filename[0]);
    if(ret) {
        printf("Unable to load AF file! Make sure the file exists and is a valid AF file.\n");
        goto exit_2;
    }
    
    // Some vars
    FILE *fp;
    char namebuf[256];
    
    // Open output file
    FILE *f;
    sprintf(namebuf, "%s/%s.html", outdir->sval[0], name->sval[0]);
    f = fopen(namebuf, "w");
    if(f == NULL) {
        printf("Error while opening file!");
        goto exit_2;
    }
    
    // Print header to file
    fprintf(f, "%s", header);
    fprintf(f, "<h1>%s</h1>", name->sval[0]);
    
    // Root
    fprintf(f, "<h2>General information</h2><table><tr><th>Key</th><th>Value</th></tr>");
    fprintf(f, "<tr><td>File ID</td><td>%d</td></tr>", af.file_id);
    fprintf(f, "<tr><td>Unknown A</td><td>%d</td></tr>", af.unknown_a);
    fprintf(f, "<tr><td>Endurance</td><td>%d</td></tr>", af.endurance);
    fprintf(f, "<tr><td>Unknown B</td><td>%d</td></tr>", af.unknown_b);
    fprintf(f, "<tr><td>Power</td><td>%d</td></tr>", af.power);
    fprintf(f, "<tr><td>Fwd speed</td><td>%d</td></tr>", af.forward_speed);
    fprintf(f, "<tr><td>Rev speed</td><td>%d</td></tr>", af.reverse_speed);
    fprintf(f, "<tr><td>Jump speed</td><td>%d</td></tr>", af.jump_speed);
    fprintf(f, "<tr><td>Fall speed</td><td>%d</td></tr>", af.fall_speed);
    fprintf(f, "<tr><td>Unknown C</td><td>%d</td></tr>", af.unknown_c);
    fprintf(f, "</table>");
    
    // Animations
    fprintf(f, "<h2>Animations</h2><div id=\"animations\">");
    for(int m = 0; m < 70; m++) {
        if(af.moves[m]) {
            sd_move *afm = af.moves[m];
            sd_animation *ani = afm->animation;

            char anim_name[32];
            anim_name[0] = 0;
            if(m > 0 && m <= 14) {
                sprintf(anim_name, ": %s", anim_names_a[m]);
            } else if(m == 48 || m == 49) {
                sprintf(anim_name, ": %s", anim_names_b[m-48]);
            } else if(m >= 55 && m <= 57) {
                sprintf(anim_name, ": %s", anim_names_c[m-55]);
            }


            fprintf(f, "<h3>Animation %d %s</h3><div class=\"animation\">", m, anim_name);
            fprintf(f, "<div class=\"iblock\"><h4>General information</h4>");
            fprintf(f, "<table><tr><th>Key</th><th>Value</th></tr>");
            fprintf(f, "<tr><td>Move string</td><td>\"%s\"</td></tr>", afm->move_string);
            fprintf(f, "<tr><td>Footer string</td><td>\"%s\"</td></tr>", afm->footer_string);

            fprintf(f, "<tr><td>Unknown 0</td><td>%d</td></th>", afm->unknown_0);
            fprintf(f, "<tr><td>Unknown 2</td><td>%d</td></th>", afm->unknown_2);
            fprintf(f, "<tr><td>Unknown 4</td><td>%d</td></th>", afm->unknown_4);
            fprintf(f, "<tr><td>Unknown 5</td><td>%d</td></th>", afm->unknown_5);
            fprintf(f, "<tr><td>Unknown 6</td><td>%d</td></th>", afm->unknown_6);
            fprintf(f, "<tr><td>Unknown 7</td><td>%d</td></th>", afm->unknown_7);
            fprintf(f, "<tr><td>Unknown 8</td><td>%d</td></th>", afm->unknown_8);
            fprintf(f, "<tr><td>Unknown 9</td><td>%d</td></th>", afm->unknown_9);
            fprintf(f, "<tr><td>Unknown 10</td><td>%d</td></th>", afm->unknown_10);
            fprintf(f, "<tr><td>Unknown 11</td><td>%d</td></th>", afm->unknown_11);
            fprintf(f, "<tr><td>Next animation ID</td><td>%d</td></th>", afm->next_anim_id);
            fprintf(f, "<tr><td>Category</td><td>%d</td></th>", afm->category);
            fprintf(f, "<tr><td>Unknown 14</td><td>%d</td></th>", afm->unknown_14);
            fprintf(f, "<tr><td>Scrap amount</td><td>%d</td></th>", afm->scrap_amount);
            fprintf(f, "<tr><td>Successor ID</td><td>%d</td></th>", afm->successor_id);
            fprintf(f, "<tr><td>Damage amount</td><td>%d</td></th>", afm->damage_amount);
            fprintf(f, "<tr><td>Unknown 18</td><td>%d</td></th>", afm->unknown_18);
            fprintf(f, "<tr><td>Unknown 19</td><td>%d</td></th>", afm->unknown_19);
            fprintf(f, "<tr><td>Points on hit</td><td>%d</td></th>", afm->points);

            fprintf(f, "<tr><td>Start X</td><td>%d</td></tr>", ani->start_x);
            fprintf(f, "<tr><td>Start Y</td><td>%d</td></tr>", ani->start_y);
            fprintf(f, "<tr><td>Animation string</td><td>\"%s\"</td></tr>", ani->anim_string);
            fprintf(f, "<tr><td>null</td><td>%d</td></tr>", ani->null);

            fprintf(f, "</table></div>");
            
            // Extra strings
            if(ani->extra_string_count) {
                fprintf(f, "<div class=\"iblock\"><h4>Extra strings</h4>");
                fprintf(f, "<table><tr><th>#</th><th>String</th></tr>");
                for(int e = 0; e < ani->extra_string_count; e++) {
                    fprintf(f, "<tr><td>%d</td><td>\"%s\"</td></tr>", e, ani->extra_strings[e]);
                }
                fprintf(f, "</table></div>");
            }
            
            // Coords
            if(ani->coord_count > 0) {
                fprintf(f, "<div class=\"iblock\"><h4>Collision coordinates</h4>");
                fprintf(f, "<table><tr><th>X</th><th>Y</th><th>null</th><th>frame_id</th></tr>");
                for(int c = 0; c < ani->coord_count; c++) {
                    sd_coord *coord = &ani->coord_table[c];
                    fprintf(f, "<tr><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>", coord->x, coord->y, coord->null, coord->frame_id);
                }
                fprintf(f, "</table></div>");
            }
            
            // Frames
            fprintf(f, "<div class=\"iblock\"><h4>Frames</h4>");
            fprintf(f, "<table><tr><th>#</th><th>A-Z</th><th>X</th><th>Y</th><th>W</th><th>H</th><th>Index</th><th>Missing</th><th>Sprite</th></tr>");
            sd_rgba_image img;
            for(int b = 0; b < ani->sprite_count; b++) {
                sd_sprite *sprite = ani->sprites[b];
                
                // Write sprite
                if(sprite->len > 0 && sprite->width > 0 && sprite->height > 0) {
                    sprintf(namebuf, "%s/%s_sprite_%d_%d.png", outdir->sval[0], name->sval[0], m, b);
                    fp = fopen(namebuf, "wb");
                    sd_sprite_rgba_decode(&img, sprite, bk.palettes[0], 0);
                    write_png(fp, img.data, img.w, img.h);
                    sd_rgba_image_free(&img);
                    fclose(fp);
                    sprintf(namebuf, "%s_sprite_%d_%d.png", name->sval[0], m, b);
                } else {
                    namebuf[0] = 0;
                }
                
                // Print html
                fprintf(f, "<tr><td>%d<td>%c</td></td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td><img src=\"%s\" /></td></tr>", 
                    b,
                    b + 'A',
                    sprite->pos_x,
                    sprite->pos_y,
                    sprite->width,
                    sprite->height,
                    sprite->index,
                    sprite->missing,
                    namebuf);
            }
            fprintf(f, "</table>");
            
            fprintf(f, "</div></div>");
        }
    }
    fprintf(f, "</div>");
    
    // Sounds
    fprintf(f, "<h2>Sound table</h2><table><tr><th>Local ID</th><th>Sound ID</th></tr>");
    for(int i = 0; i < 30; i++) {
        fprintf(f, "<tr><td>%d</td><td>%d</td></tr>", i, (int)af.soundtable[i]);
    }
    fprintf(f, "</table>");
    
    
    // Print footer to file
    fprintf(f, "%s", footer);
    
    // Quit
    fclose(f);
exit_2:
    sd_af_free(&af);
exit_1:
    sd_bk_free(&bk);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
