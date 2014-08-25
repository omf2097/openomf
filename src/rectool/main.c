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
#include "../shared/pilot.h"

const char* mstr[] = {
    "PUNCH",
    "KICK",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT"
};

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

void print_rec_root_info(sd_rec_file *rec) {
    if(rec != NULL) {
        // Print enemy data
        printf("Enemies:\n");
        for(int i = 0; i < 2; i++) {
            print_pilot_info(&rec->pilots[i]);
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
        printf("Number Tick       Extra Player Action        Action enum  Extra data\n");
        for(int i = 0; i < rec->move_count; i++) {
            char tmp[100];
            tmp[0] = 0;
            if(rec->moves[i].extra < 3) {
                print_key(tmp, rec->moves[i].action);
            }
            printf("%6u %10u %5u %6u %6u %18s",
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
    unsigned int action = atoi(value);
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
            rec->moves[entry_id].action = SD_REC_NONE;
            if(action & SD_REC_PUNCH) {
                rec->moves[entry_id].action |= SD_REC_PUNCH;
            }
            if(action & SD_REC_KICK) {
                rec->moves[entry_id].action |= SD_REC_KICK;
            }
            switch(action & 0xF0) {
                case 16: rec->moves[entry_id].action |= SD_REC_UP; break;
                case 32: rec->moves[entry_id].action |= (SD_REC_UP|SD_REC_RIGHT); break;
                case 48: rec->moves[entry_id].action |= SD_REC_RIGHT; break;
                case 64: rec->moves[entry_id].action |= (SD_REC_DOWN|SD_REC_RIGHT); break;
                case 80: rec->moves[entry_id].action |= SD_REC_DOWN; break;
                case 96: rec->moves[entry_id].action |= (SD_REC_DOWN|SD_REC_LEFT); break;
                case 112: rec->moves[entry_id].action |= SD_REC_LEFT; break;
                case 128: rec->moves[entry_id].action |= (SD_REC_UP|SD_REC_LEFT); break;
            }
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
    struct arg_int *delete = arg_intn("d", "delete", "<number>", 0, 10, "Delete an existing element");
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
    } else if (delete->count > 0) {
        int last = -1;
        int offset = 0;
        for(int i = 0; i < delete->count; i++) {
            if (delete->ival[i] < last) {
                printf("Can't delete out of order, please list deletes in ascending order! %d %d\n", delete->ival[i], last);
                goto exit_1;
            }
            if (sd_rec_delete_action(&rec, delete->ival[i] - offset) != SD_SUCCESS) {
                printf("deleting move %d failed\n", delete->ival[i]);
            }
            last = delete->ival[i];
            offset++;
        }
        printf("Deleted %d moves, final move count %d\n", offset, rec.move_count);
    } else if(insert->count > 0) {
        sd_rec_move mv;
        memset(&mv, 0, sizeof(sd_rec_move));
        if(sd_rec_insert_action(&rec, insert->ival[0], &mv) != SD_SUCCESS) {
            printf("Inserting move to slot %d failed.", insert->ival[0]);
        } else {
            printf("Inserted move to slot %d.", insert->ival[0]);
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
