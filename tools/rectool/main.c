/** @file main.c
 * @brief .REC file editor tool
 * @license MIT
 */

#include "../shared/pilot.h"
#include "formats/error.h"
#include "formats/rec.h"
#include "formats/rec_assertion.h"
#include "utils/c_array_util.h"
#if defined(ARGTABLE2_FOUND)
#include <argtable2.h>
#elif defined(ARGTABLE3_FOUND)
#include <argtable3.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *mstr[] = {"PUNCH", "KICK", "UP", "DOWN", "LEFT", "RIGHT"};

void print_key(char *o, uint8_t key) {
    int pos = 0;
    o[0] = 0;
    for(int i = 0; i < 6; i++) {
        uint8_t m = 1 << i;
        if(key & m) {
            if(pos > 0) {
                pos += sprintf((char *)(o + pos), "|");
            }
            pos += sprintf((char *)(o + pos), "%s", mstr[i]);
        }
    }
}

/*
static const int tmp_lengths[] = {
    2,1,1,5,5,5,1,2,2,1
};
*/

static const char *onoff[] = {"Off", "On"};

static const char *match_type[] = {"One match", "2 out of 3", "3 out of 5", "4 out of 7"};

static const char *knockdown_text[] = {"None", "Kicks", "Punches", "Both"};

static const char *game_mode_text[] = {"?", "Tournament", "1/2 player"};

static const char *controller_text[] = {"?",  "1P Custom", "2P Custom",     "Joystick 1",     "Joystick 2",
                                        "AI", "Network",   "Left Keyboard", "Right Keyboard", "Replay"};

void print_rec_root_info(sd_rec_file *rec) {
    if(rec != NULL) {
        // Print enemy data
        printf("## Pilots:\n");
        print_pilot_array_header();
        for(int i = 0; i < 2; i++) {
            print_pilot_array_row(&rec->pilots[i].info, i);
        }
        printf("\n");

        printf("## Unknown header data:\n");
        printf("  - Score A:      %d\n", rec->scores[0]);
        printf("  - Score B:      %d\n", rec->scores[1]);
        printf("  - Unknown A:    %d\n", rec->unknown_a);
        printf("  - Unknown B:    %d\n", rec->unknown_b);
        printf("  - Game Mode:    %s\n", game_mode_text[rec->game_mode]);
        printf("  - Throw Range:  %d\n", rec->throw_range);
        printf("  - Hit Pause:    %d\n", rec->hit_pause);
        printf("  - Block Damage: %d\n", rec->block_damage);
        printf("  - Vitality:     %d\n", rec->vitality);
        printf("  - Jump Height:  %d\n", rec->jump_height);
        printf("  - P1 Control:   %s\n", controller_text[rec->p1_controller]);
        printf("  - P2 Control:   %s\n", controller_text[rec->p2_controller]);
        printf("  - P2 Control:   %s\n", controller_text[rec->p2_controller_]);
        printf("  - Knock down:   %s\n", knockdown_text[rec->knock_down]);
        printf("  - Rehit mode:   %s\n", onoff[rec->rehit_mode]);
        printf("  - Def. throws:  %s\n", onoff[rec->def_throws]);
        printf("  - Arena ID:     %d\n", rec->arena_id);
        printf("  - Power 1:      %d\n", rec->power[0]);
        printf("  - Power 2:      %d\n", rec->power[1]);
        printf("  - Hazards:      %s\n", onoff[rec->hazards]);
        printf("  - Round type:   %s\n", match_type[rec->round_type]);
        printf("  - Unknown L:    %d\n", rec->unknown_l);
        printf("  - Hyper Mode:   %s\n", onoff[rec->hyper_mode]);
        printf("  - Unknown M:    %d\n", rec->unknown_m);
        printf("\n");

        printf("## Parsed data:\n");
        printf("Number Tick       Extra Player Action Length            Action enum Extra data\n");
        for(unsigned i = 0; i < rec->move_count; i++) {
            char tmp[100];
            tmp[0] = 0;
            if(rec->moves[i].lookup_id < 3) {
                print_key(tmp, rec->moves[i].action);
            }
            printf("%6u %10u %5u %6u %6u %6u %22s", i, rec->moves[i].tick, rec->moves[i].lookup_id,
                   rec->moves[i].player_id, rec->moves[i].raw_action, sd_rec_extra_len(rec->moves[i].lookup_id), tmp);

            if(rec->moves[i].lookup_id == 10) {
                uint8_t buf[8];
                buf[0] = rec->moves[i].raw_action;
                memcpy(buf + 1, rec->moves[i].extra_data, 7);
                rec_assertion ass;
                if(parse_assertion(buf, &ass)) {
                    print_assertion(&ass);
                }
            } else if(rec->moves[i].lookup_id > 2) {
                print_bytes(rec->moves[i].extra_data, sd_rec_extra_len(rec->moves[i].lookup_id) - 1, 8, 2);
            }
            printf("\n");
        }
    }
}

int rec_entry_key_get_id(const char *key) {
    if(strcmp(key, "tick") == 0)
        return 0;
    if(strcmp(key, "lookup_id") == 0)
        return 1;
    if(strcmp(key, "player_id") == 0)
        return 2;
    if(strcmp(key, "action") == 0)
        return 3;
    if(strcmp(key, "extra_data") == 0)
        return 4;
    return -1;
}

rec_assertion_operator rec_assertion_get_operator(const char *key) {
    if(strcmp(key, "gt") == 0)
        return OP_GT;
    if(strcmp(key, "lt") == 0)
        return OP_LT;
    if(strcmp(key, "eq") == 0)
        return OP_EQ;

    return OP_INVALID;
}

rec_har_attr rec_assertion_get_har_attr(const char *key) {
    if(strcmp(key, "xpos") == 0)
        return ATTR_X_POS;
    if(strcmp(key, "ypos") == 0)
        return ATTR_Y_POS;
    if(strcmp(key, "xvel") == 0)
        return ATTR_X_VEL;
    if(strcmp(key, "yvel") == 0)
        return ATTR_Y_VEL;
    if(strcmp(key, "state") == 0)
        return ATTR_STATE_ID;
    if(strcmp(key, "anim") == 0)
        return ATTR_ANIMATION_ID;
    if(strcmp(key, "health") == 0)
        return ATTR_HEALTH;
    if(strcmp(key, "stamina") == 0)
        return ATTR_STAMINA;

    return ATTR_INVALID;
}

int rec_assertion_get_operand(rec_assertion_operand *op, const char *operand, const char *value) {
    if(strcmp(operand, "har1") == 0) {
        op->is_literal = false;
        op->value.attr.har_id = 0;
        op->value.attr.attribute = rec_assertion_get_har_attr(value);
        if(op->value.attr.attribute == ATTR_INVALID) {
            return 1;
        }
        return 0;
    } else if(strcmp(operand, "har2") == 0) {
        op->is_literal = false;
        op->value.attr.har_id = 1;
        op->value.attr.attribute = rec_assertion_get_har_attr(value);
        if(op->value.attr.attribute == ATTR_INVALID) {
            return 1;
        }
        return 0;
    } else if(strcmp(operand, "literal") == 0) {
        op->is_literal = true;
        op->value.literal = atoi(value);
        return 0;
    }

    return 1;
}

void rec_entry_set_key(sd_rec_file *rec, int entry_id, const char *key, const char *value) {
    unsigned int action = atoi(value);
    switch(rec_entry_key_get_id(key)) {
        case 0:
            rec->moves[entry_id].tick = atoi(value);
            break;
        case 1:
            rec->moves[entry_id].lookup_id = atoi(value);
            break;
        case 2:
            rec->moves[entry_id].player_id = atoi(value);
            break;
        case 3:
            rec->moves[entry_id].action = SD_ACT_NONE;
            if(action & SD_ACT_PUNCH) {
                rec->moves[entry_id].action |= SD_ACT_PUNCH;
            }
            if(action & SD_ACT_KICK) {
                rec->moves[entry_id].action |= SD_ACT_KICK;
            }
            switch(action & 0xF0) {
                case 16:
                    rec->moves[entry_id].action |= SD_ACT_UP;
                    break;
                case 32:
                    rec->moves[entry_id].action |= (SD_ACT_UP | SD_ACT_RIGHT);
                    break;
                case 48:
                    rec->moves[entry_id].action |= SD_ACT_RIGHT;
                    break;
                case 64:
                    rec->moves[entry_id].action |= (SD_ACT_DOWN | SD_ACT_RIGHT);
                    break;
                case 80:
                    rec->moves[entry_id].action |= SD_ACT_DOWN;
                    break;
                case 96:
                    rec->moves[entry_id].action |= (SD_ACT_DOWN | SD_ACT_LEFT);
                    break;
                case 112:
                    rec->moves[entry_id].action |= SD_ACT_LEFT;
                    break;
                case 128:
                    rec->moves[entry_id].action |= (SD_ACT_UP | SD_ACT_LEFT);
                    break;
            }
            break;
        default:
            printf("Invalid record entry key!\n");
            return;
    }
}

void rec_entry_get_key(sd_rec_file *rec, int entry_id, const char *key) {
    switch(rec_entry_key_get_id(key)) {
        case 0:
            printf("%d", rec->moves[entry_id].tick);
            break;
        case 1:
            printf("%d", rec->moves[entry_id].lookup_id);
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

int rec_key_get_id(const char *key) {
    if(strcmp(key, "entry") == 0)
        return 0;
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
            printf("Unknown key %s!\n", key[0]);
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
                if(r >= (int)rec->move_count) {
                    printf("Index does not exist.");
                    return;
                }
                char tmp[100];
                tmp[0] = 0;
                if(rec->moves[r].lookup_id < 3) {
                    print_key(tmp, rec->moves[r].action);
                }
                printf("Tick:       %d\n", rec->moves[r].tick);
                printf("Extra:      %d\n", rec->moves[r].lookup_id);
                printf("Player ID:  %d\n", rec->moves[r].player_id);
                printf("Action:     %s\n", tmp);
                if(rec->moves[r].extra_data) {
                    printf("Extra data: ");
                    print_bytes(rec->moves[r].extra_data, 7, 7, 0);
                }
                return;
            }
            if(kcount == 3) {
                int entry_id = atoi(key[1]);
                rec_entry_get_key(rec, entry_id, key[2]);
                return;
            }
        } break;
        default:
            printf("Unknown key %s!\n", key[0]);
    }
}

int main(int argc, char *argv[]) {
    // commandline argument parser options
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Input .REC file");
    struct arg_file *output = arg_file0("o", "output", "<file>", "Output .REC file");
    struct arg_str *key = arg_strn("k", "key", "<key>", 0, 3, "Select key");
    struct arg_int *pilot = arg_int0(NULL, "pilot", "<int>", "Only print pilot information");
    struct arg_str *value = arg_str0("s", "set", "<value>", "Set value (requires --key)");
    struct arg_int *insert = arg_int0("i", "insert", "<number>", "Insert a new element");
    struct arg_int *assert = arg_int0(NULL, "assert", "<number>", "Insert an assertion at <number>");
    struct arg_int *assert_tick = arg_int0(NULL, "assertion_tick", "<number>", "Tick at which to do assertion");
    struct arg_str *assert_op = arg_str0(NULL, "operator", "<gt|lt|eq>", "Assertion operator");
    struct arg_str *assert_op1 =
        arg_str0(NULL, "operand1", "<har1|har2|literal>", "Assertion operand 1; har 1 or 2 or literal");
    struct arg_str *assert_val1 =
        arg_str0(NULL, "operand1_value", "<xpos|ypos|xvel|yvel|state|anim|health|stamina|<literal>>",
                 "Assertion operand 1; har 1 or 2 or literal");
    struct arg_str *assert_op2 =
        arg_str0(NULL, "operand2", "<har1|har2|literal>", "Assertion operand 2; har 1 or 2 or literal");
    struct arg_str *assert_val2 =
        arg_str0(NULL, "operand2_value", "<xpos|ypos|xvel|yvel|state|anim|health|stamina|<literal>>",
                 "Assertion operand 1; har 1 or 2 or literal");
    struct arg_int *delete = arg_intn("d", "delete", "<number>", 0, 10, "Delete an existing element");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help,       vers,        file,       output,      pilot,       key,
                        value,      delete,      insert,     assert,      assert_tick, assert_op,
                        assert_op1, assert_val1, assert_op2, assert_val2, end};
    const char *progname = "rectool";

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
    } else if(delete->count > 0) {
        int last = -1;
        int offset = 0;
        for(int i = 0; i < delete->count; i++) {
            if(delete->ival[i] < last) {
                printf("Can't delete out of order, please list deletes in ascending order! %d %d\n", delete->ival[i],
                       last);
                goto exit_1;
            }
            if(sd_rec_delete_action(&rec, delete->ival[i] - offset) != SD_SUCCESS) {
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
            printf("Inserting move to slot %d failed.\n", insert->ival[0]);
        } else {
            printf("Inserted move to slot %d.\n", insert->ival[0]);
        }
    } else if(assert->count > 0) {
        if(assert_op->count == 1 && assert_op1->count == 1 && assert_val1->count == 1 && assert_op2->count == 1 &&
           assert_val2->count == 1) {

            rec_assertion op;

            op.op = rec_assertion_get_operator(assert_op->sval[0]);

            if(op.op == OP_INVALID) {
                printf("invalid operator %s\n", assert_op->sval[0]);
                goto exit_1;
            }

            if(rec_assertion_get_operand(&op.operand1, assert_op1->sval[0], assert_val1->sval[0])) {
                printf("invalid operand %s %s\n", assert_op1->sval[0], assert_val1->sval[0]);
                goto exit_1;
            }

            if(rec_assertion_get_operand(&op.operand2, assert_op2->sval[0], assert_val2->sval[0])) {
                printf("invalid operand %s %s\n", assert_op2->sval[0], assert_val2->sval[0]);
                goto exit_1;
            }

            print_assertion(&op);

            uint8_t buf[8];
            if(!encode_assertion(&op, buf)) {
                printf("failed to encode assertion\n");
                goto exit_1;
            }

            sd_rec_move mv;
            memset(&mv, 0, sizeof(sd_rec_move));
            mv.lookup_id = 10;
            mv.raw_action = buf[0];
            mv.extra_data = malloc(7);
            if(assert_tick->count > 0) {
                mv.tick = assert_tick->ival[0];
            }
            memcpy(mv.extra_data, buf + 1, 7);
            if(sd_rec_insert_action(&rec, assert->ival[0], &mv) != SD_SUCCESS) {
                printf("Inserting move to slot %d failed.\n", assert->ival[0]);
            } else {
                printf("Inserted move to slot %d.\n", assert->ival[0]);
            }
        } else {
            printf("incomplete assert.\n");
            goto exit_1;
        }
    } else if(pilot->count > 0) {
        int i = pilot->ival[0];
        if(i < 0 || i > 1) {
            printf("Pilot ID out of bounds.\n");
            goto exit_1;
        }
        print_pilot_info(&rec.pilots[i].info);
        printf("\n");
        printf("  - Unknown: %d\n", rec.pilots[i].unknown_a);
        printf("  - Unknown: %d\n", rec.pilots[i].unknown_b);
        printf("  - Palette:\n");
        print_bytes((char *)rec.pilots[i].pal.colors, 144, 16, 4);
        printf("\n");

        if(rec.pilots[i].has_photo) {
            printf("  - Photo len  = %d\n", rec.pilots[i].photo.len);
            printf("  - Photo size = (%d,%d)\n", rec.pilots[i].photo.width, rec.pilots[i].photo.height);
            printf("  - Photo pos  = (%d,%d)\n", rec.pilots[i].photo.pos_x, rec.pilots[i].photo.pos_y);
            printf("  - Missing    = %d\n", rec.pilots[i].photo.missing);
            printf("  - Index      = %d\n", rec.pilots[i].photo.index);
        } else {
            printf("  - No photo.\n");
        }
        printf("\n");
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
    arg_freetable(argtable, N_ELEMENTS(argtable));
    return 0;
}
