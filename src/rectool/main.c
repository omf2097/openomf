/** @file main.c
  * @brief .REC file editor tool
  * @license MIT
  */

#include <argtable2.h>
#include <shadowdive/shadowdive.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

void print_rec_root_info(sd_rec_file *rec) {
    if(rec != NULL) {
        // Print enemy data
        printf("Enemies:\n");
        for(int i = 0; i < 2; i++) {
            print_pilot_info(rec->pilots[i]);
        }

        char tmp = 'A';
        printf("## Unknown header data:\n");
        printf("  - Score A: %d\n", rec->scores[0]);
        printf("  - Score B: %d\n", rec->scores[1]);
        printf("  - %c:       %d\n", tmp++, rec->unknown_a);
        printf("  - %c:       %d\n", tmp++, rec->unknown_b);
        printf("  - %c:       %d\n", tmp++, rec->unknown_c);
        printf("  - %c:       %d\n", tmp++, rec->unknown_d);
        printf("  - %c:       %d\n", tmp++, rec->unknown_e);
        printf("  - %c:       %d\n", tmp++, rec->unknown_f);
        printf("  - %c:       %d\n", tmp++, rec->unknown_g);
        printf("  - %c:       %d\n", tmp++, rec->unknown_h);
        printf("  - %c:       %d\n", tmp++, rec->unknown_i);
        printf("  - %c:       %d\n", tmp++, rec->unknown_j);
        printf("  - %c:       %d\n", tmp++, rec->unknown_k);
        printf("  - %c:       %d\n", tmp++, rec->unknown_l);
        printf("  - %c:       %d\n", tmp++, rec->unknown_m);
        printf("\n");

        printf("## Parsed data:\n");
        printf("Number   Tick Extra Player Action        Action enum  Extra data\n");
        for(int i = 0; i < rec->move_count; i++) {
            char tmp[100];
            tmp[0] = 0;
            if(rec->moves[i].extra < 3) {
                print_key(tmp, rec->moves[i].action);
            }
            printf(" - %3d: %5d %5d %6d %6d %18s",
                i,
                rec->moves[i].tick,
                rec->moves[i].extra,
                rec->moves[i].player_id,
                rec->moves[i].raw_action,
                tmp);

            if(rec->moves[i].extra > 2) {
                print_bytes(rec->moves[i].extra_data, 7, 8, 2);
            }
            printf("\n");
        }
    }
}

int rec_entry_key_get_id(const char* key) {
    if(strcmp(key, "tick") == 0) return 0;
    if(strcmp(key, "extra") == 0) return 1;
    if(strcmp(key, "player_id") == 0) return 2;
    if(strcmp(key, "action") == 0) return 3;
    if(strcmp(key, "extra_data") == 0) return 4;
    return -1;
}

void rec_entry_set_key(sd_rec_file *rec, int entry_id, const char *key, const char *value) {
    switch(rec_entry_key_get_id(key)) {
        case 0:
            rec->moves[entry_id].tick = atoi(value);
            break;
        case 1:
            rec->moves[entry_id].extra = atoi(value);
            break;
        case 2:
            rec->moves[entry_id].player_id = atoi(value);
            break;
        case 3:
            rec->moves[entry_id].action = atoi(value);
            break;
        default:
            printf("Invalid record entry key!\n");
            return;
    }
}

void rec_entry_get_key(sd_rec_file *rec, int entry_id, const char* key) {
    switch(rec_entry_key_get_id(key)) {
        case 0:
            printf("%d", rec->moves[entry_id].tick);
            break;
        case 1:
            printf("%d", rec->moves[entry_id].extra);
            break;
        case 2:
            printf("%d", rec->moves[entry_id].player_id);
            break;
        case 3: {
            char tmp[100];
            print_key(tmp, rec->moves[entry_id].action);
            printf("%s", tmp);
            } break;
        case 4:
            print_bytes(rec->moves[entry_id].extra_data, 7, 8, 0);
            break;
        default:
            printf("Invalid record entry key!\n");
            return;
    }
}

int rec_key_get_id(const char* key) {
    if(strcmp(key, "entry") == 0) return 0;
    return -1;
}

void rec_set_key(sd_rec_file *rec, const char **key, int kcount, const char *value) {
    switch(rec_key_get_id(key[0])) {
        case 0: {
            if(kcount == 1) {
                printf("Record ID required!\n");
                return;
            }
            if(kcount == 2) {
                printf("Record key ID required\n");
                return;
            }
            if(kcount == 3) {
                int entry_id = atoi(key[1]);
                rec_entry_set_key(rec, entry_id, key[2], value);
                return;
            }
            } break;
        default:
            printf("Unknown key!\n");
    }
}

void rec_get_key(sd_rec_file *rec, const char **key, int kcount) {
    switch(rec_key_get_id(key[0])) {
        case 0: {
            if(kcount == 1) {
                printf("Record ID required!\n");
                return;
            }
            if(kcount == 2) {
                int r = atoi(key[1]);
                if(r >= rec->move_count) {
                    printf("Index does not exist.");
                    return;
                }
                char tmp[100];
                tmp[0] = 0;
                if(rec->moves[r].extra < 3) {
                    print_key(tmp, rec->moves[r].action);
                }
                printf("Tick:       %d\n", rec->moves[r].tick);
                printf("Extra:      %d\n", rec->moves[r].extra);
                printf("Player ID:  %d\n", rec->moves[r].player_id);
                printf("Action:     %s\n", tmp);
                printf("Extra data: ");
                print_bytes(rec->moves[r].extra_data, 7, 7, 0);
                return;
            }
            if(kcount == 3) {
                int entry_id = atoi(key[1]);
                rec_entry_get_key(rec, entry_id, key[2]);
                return;
            }
            } break;
        default:
            printf("Unknown key!\n");
    }
}


int main(int argc, char* argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .REC file");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 3, "Select key");
    struct arg_str *value = arg_str0("s", "set", "<value>", "Set value (requires --key)");
    struct arg_int *insert = arg_int0("i", "insert", "<number>", "Insert a new element");
    struct arg_int *delete = arg_int0("d", "delete", "<number>", "Delete an existing element");
    struct arg_end *end = arg_end(20);
    void* argtable[] = {help,vers,file,output,key,value,delete,insert,end};
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

    // Note about needing --output when changing values
    if(value->count > 0 && output->count <= 0) {
        printf("For setting values, remember to set --output or -o.\n");
        goto exit_0;
    }
    
    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stdout, end, progname);
        printf("Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Make sure delete and insert aren't both selected
    if(delete->count > 0 && insert->count > 0) {
        printf("Select either --delete or --insert, not both!");
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

    // Check if we want to fetch/set specific values
    if(key->count > 0) {
        if(value->count > 0) {
            rec_set_key(&rec, key->sval, key->count, value->sval[0]);
        } else {
            rec_get_key(&rec, key->sval, key->count);
        }
    } else {
        print_rec_root_info(&rec);
    }
    
    // Write output file
    if(output->count > 0) {
        if(sd_rec_save(&rec, output->filename[0]) != SD_SUCCESS) {
            printf("Save didn't succeed!");
        }
    }
    
    // Quit
exit_1:
    sd_rec_free(&rec);
exit_0:
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
    return 0;
}
