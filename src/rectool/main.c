/** @file main.c
  * @brief .REC file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

const char* mstr[] = {
    "PUNCH",
    "KICK",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT"
};

void print_bytes(char *buf, int len, int line, int padding) {
    for(int k = 0; k < padding; k++) {
        printf(" ");
    }
    for(int i = 1; i <= len; i++) {
        printf("%02x ", (uint8_t)buf[i-1]);
        if(i % line == 0) {
            printf("\n");
            for(int k = 0; k < padding; k++) {
                printf(" ");
            }
        }
    }
}

void print_key(char *o, uint8_t key) {
    int pos = 0;
    o[0] = 0;
    for(int i = 0; i < 6; i++) {
        uint8_t m = 1 << i;
        if(key & m) {
            if(pos > 0) {
                pos += sprintf((char*)(o+pos), "|");
            }
            pos += sprintf((char*)(o+pos), "%s", mstr[i]);
        }
    }
}

void print_pilot_info(sd_pilot *pilot) {
    if(pilot != NULL) {
        printf("### Pilot header for %s:\n", pilot->name);
        printf("  - Wins:        %d\n", pilot->wins);
        printf("  - Losses:      %d\n", pilot->losses);
        printf("  - Robot ID:    %d\n", pilot->robot_id);
        printf("  - Offense:     %d\n", pilot->offense);
        printf("  - Defense:     %d\n", pilot->defense);
        printf("  - Money:       %d\n", pilot->money);
        printf("  - Color:       %d,%d,%d\n", 
            pilot->color_1,
            pilot->color_2,
            pilot->color_3);

        printf("  - Stats:       ");
        print_bytes(pilot->stats, 8, 10, 0);
        printf("\n");
        printf("  - unk_block_a:\n");
        print_bytes(pilot->unk_block_a, 107, 16, 5);
        printf("\n");
        printf("  - Force arena: %d\n", pilot->force_arena);
        printf("  - unk_block_b: ");
        print_bytes(pilot->unk_block_a, 3, 16, 0);
        printf("\n");
        printf("  - Movement:    %d\n", pilot->movement);
        printf("  - unk_block_c: ");
        print_bytes(pilot->unk_block_c, 6, 16, 0);
        printf("\n");
        printf("  - Enhancement: ");
        print_bytes(pilot->enhancements, 11, 16, 0);
        printf("\n");
        printf("  - Flags:       %d\n", pilot->flags);

        printf("  - Reqs:        ");
        print_bytes((char*)pilot->reqs, 10, 16, 0);
        printf("\n");
        printf("  - Attitude:    ");
        print_bytes((char*)pilot->attitude, 6, 16, 0);
        printf("\n");
        printf("  - unk_block_d: ");
        print_bytes(pilot->unk_block_d, 6, 16, 0);
        printf("\n");

        printf("  - AP Throw:    %d\n", pilot->ap_throw);
        printf("  - AP Special:  %d\n", pilot->ap_special);
        printf("  - AP Jump:     %d\n", pilot->ap_jump);
        printf("  - AP High:     %d\n", pilot->ap_high);
        printf("  - AP Low:      %d\n", pilot->ap_low);
        printf("  - AP Middle:   %d\n", pilot->ap_middle);

        printf("  - Pref jump    %d\n", pilot->pref_jump);
        printf("  - Pref fwd     %d\n", pilot->pref_fwd);
        printf("  - Pref back    %d\n", pilot->pref_back);

        printf("  - unk_block_e: ");
        print_bytes(pilot->unk_block_d, 4, 16, 0);
        printf("\n");

        printf("  - Learning     %f\n", pilot->learning);
        printf("  - Forget       %f\n", pilot->forget);
        printf("  - Winnings     %d\n", pilot->winnings);
        printf("  - Photo ID     %d\n", pilot->photo_id);

        printf("  - unk_block_f: ");
        print_bytes(pilot->unk_block_f, 24, 26, 0);
        printf("\n");
        printf("  - unk_block_g:\n");
        print_bytes(pilot->unk_block_g, 166, 16, 5);
        printf("\n\n");
    }
}

int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .REC file");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,end};
    const char* progname = "rectool";
    
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
        printf("Command line One Must Fall 2097 .REC file editor.\n");
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
    
    // Load file
    sd_rec_file rec;
    sd_rec_create(&rec);
    if(file->count > 0) {
        int ret = sd_rec_load(&rec, file->filename[0]);
        if(ret != SD_SUCCESS) {
            printf("Unable to load REC file! [%d] %s.\n", ret, sd_get_error(ret));
            goto exit_1;
        }
    }

    // Print enemy data
    printf("Enemies:\n");
    for(int i = 0; i < 2; i++) {
        print_pilot_info(rec.pilots[i]);
    }

    char tmp = 'A';
    printf("## Unknown header data:\n");
    printf("  - Score A: %d\n", rec.scores[0]);
    printf("  - Score B: %d\n", rec.scores[1]);
    printf("  - %c:       %d\n", tmp++, rec.unknown_a);
    printf("  - %c:       %d\n", tmp++, rec.unknown_b);
    printf("  - %c:       %d\n", tmp++, rec.unknown_c);
    printf("  - %c:       %d\n", tmp++, rec.unknown_d);
    printf("  - %c:       %d\n", tmp++, rec.unknown_e);
    printf("  - %c:       %d\n", tmp++, rec.unknown_f);
    printf("  - %c:       %d\n", tmp++, rec.unknown_g);
    printf("  - %c:       %d\n", tmp++, rec.unknown_h);
    printf("  - %c:       %d\n", tmp++, rec.unknown_i);
    printf("  - %c:       %d\n", tmp++, rec.unknown_j);
    printf("  - %c:       %d\n", tmp++, rec.unknown_k);
    printf("  - %c:       %d\n", tmp++, rec.unknown_l);
    printf("  - %c:       %d\n", tmp++, rec.unknown_m);
    printf("\n");

    printf("## Parsed data:\n");
    printf("Number   Tick Extra Player Action        Action enum  Extra data\n");
    for(int i = 0; i < rec.move_count; i++) {
        char tmp[100];
        tmp[0] = 0;
        if(rec.moves[i].extra < 3) {
            print_key(tmp, rec.moves[i].action);
        }
        printf(" - %3d: %5d %5d %6d %6d %18s",
            i,
            rec.moves[i].tick,
            rec.moves[i].extra,
            rec.moves[i].player_id,
            rec.moves[i].raw_action,
            tmp);

        if(rec.moves[i].extra > 2) {
            print_bytes(rec.moves[i].extra_data, 7, 8, 2);
        }
        printf("\n");
    }
    
    // Write output file
    if(output->count > 0) {
        sd_rec_save(&rec, output->filename[0]);
    }
    
    // Quit
exit_1:
    sd_rec_free(&rec);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
